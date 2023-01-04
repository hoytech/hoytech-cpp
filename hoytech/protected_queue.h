#pragma once

#include <deque>
#include <condition_variable>
#include <thread>
#include <vector>


namespace hoytech {

template <typename T>
class protected_queue {
  public:

    // Adding items

    void push(const T& data) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            q_.push_back(data);
        }

        cv_.notify_one();
    }

    void push_move(T&& data) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            q_.emplace_back(std::move(data));
        }

        cv_.notify_one();
    }

    void push_all(std::vector<T> &vec) {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            for (auto &e : vec) {
                q_.push_back(e);
            }
        }

        cv_.notify_one();
    }

    void push_move_all(std::vector<T> &vec) {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            for (auto &e : vec) {
                q_.emplace_back(std::move(e));
            }
        }

        cv_.notify_one();
    }

    void unshift(const T& data) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            q_.push_front(data);
        }

        cv_.notify_one();
    }

    void unshift_move(T&& data) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            q_.emplace_front(std::move(data));
        }

        cv_.notify_one();
    }


    // Waiting on items

    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this](){ return q_.size() != 0; });
    }


    // Removing items

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this](){ return q_.size() != 0; });

        T data = std::move(q_.back());
        q_.pop_back();

        return data;
    }

    T shift() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this](){ return q_.size() != 0; });

        T data = std::move(q_.front());
        q_.pop_front();

        return data;
    }

    std::deque<T> pop_all() {
        decltype(q_) temp_queue;

        {
            std::unique_lock<decltype(mutex_)> lock(mutex_);
            cv_.wait(lock, [this](){ return q_.size() != 0; });

            temp_queue.swap(q_);
        }

        return temp_queue;
    }

    std::deque<T> pop_all_no_wait() {
        decltype(q_) temp_queue;

        {
            std::unique_lock<decltype(mutex_)> lock(mutex_);
            temp_queue.swap(q_);
        }

        return temp_queue;
    }

    std::deque<T> pop_all_nonblocking() {
        decltype(q_) temp_queue;

        {
            std::unique_lock<decltype(mutex_)> lock(mutex_, std::defer_lock);

            if (lock.try_lock()) {
                temp_queue.swap(q_);
            }
        }

        return temp_queue;
    }

  private:
    std::deque<T> q_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

}
