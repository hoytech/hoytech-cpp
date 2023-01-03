#pragma once

#include <queue>
#include <condition_variable>
#include <thread>
#include <functional>
#include <set>
#include <chrono>


namespace hoytech {

class timer {
  public:
    std::function<void()> setupCb; // Run in timer thread context

    void run() {
        t = std::thread([this]() {
            if (setupCb) setupCb();

            std::unique_lock<std::mutex> lock(m);

            while (1) {
                std::chrono::steady_clock::time_point wakeup;

                if (queue.empty()) {
                    wakeup = std::chrono::steady_clock::now() + std::chrono::seconds(60);
                } else {
                    wakeup = queue.top().trigger;
                }

                cv.wait_until(lock, wakeup);

                if (shutdown) return;

                while (!queue.empty()) {
                    auto &it = queue.top();
                    if (it.trigger > std::chrono::steady_clock::now()) break;

                    if (live_timers.count(it.tok)) {
                        uint64_t new_interval;

                        {
                            lock.unlock(); // in case the callback wants to add/cancel other timers
                            new_interval = it.cb();
                            lock.lock();
                        }

                        if (new_interval) {
                            auto trigger = std::chrono::steady_clock::now() + std::chrono::microseconds(new_interval);
                            queue.emplace(it.tok, it.cb, trigger);
                        } else {
                            live_timers.erase(it.tok);
                        }
                    }

                    queue.pop();
                }
            }
        });
    }

    ~timer() {
        if (!t.joinable()) return;

        std::unique_lock<std::mutex> lock(m);
        shutdown = true;

        lock.unlock();
        cv.notify_all();

        t.join();
    }

    using cancel_token = uint64_t;

    cancel_token repeat_adjustable(uint64_t interval_microseconds, std::function<uint64_t()> cb) {
        if (interval_microseconds == 0) return 0;

        std::unique_lock<std::mutex> lock(m);

        auto tok = next_cancel_token++;

        auto trigger = std::chrono::steady_clock::now() + std::chrono::microseconds(interval_microseconds);

        queue.emplace(tok, cb, trigger);
        live_timers.insert(tok);

        lock.unlock();
        cv.notify_all();

        return tok;
    }

    cancel_token repeat_adjustable(std::function<uint64_t()> cb) {
        return repeat_adjustable(1, cb);
    }

    bool cancel(cancel_token tok) {
        std::unique_lock<std::mutex> lock(m);
        return live_timers.erase(tok); // returns true if timer was cancelled
    }

    cancel_token once(uint64_t interval_microseconds, const std::function<void()> &cb) {
        return repeat_adjustable(interval_microseconds, [cb]{
            cb();
            return 0;
        });
    }

    cancel_token repeat_maybe(uint64_t interval_microseconds, const std::function<bool()> &cb) {
        return repeat_adjustable(interval_microseconds, [cb, interval_microseconds]{
            return cb() ? interval_microseconds : 0;
        });
    }

    cancel_token repeat(uint64_t interval_microseconds, const std::function<void()> &cb) {
        return repeat_adjustable(interval_microseconds, [cb, interval_microseconds]{
            cb();
            return interval_microseconds;
        });
    }

  private:
    struct item {
        item(cancel_token tok_, std::function<uint64_t()> cb_, std::chrono::steady_clock::time_point trigger_)
            : tok(tok_), cb(cb_), trigger(trigger_) {}

        bool operator<(const item& rhs) const {
            return trigger > rhs.trigger;
        }

        cancel_token tok;
        std::function<uint64_t()> cb;
        std::chrono::steady_clock::time_point trigger;
    };

    std::thread t;
    std::mutex m;
    std::condition_variable cv;
    cancel_token next_cancel_token = 1;
    std::priority_queue<item> queue;
    std::set<cancel_token> live_timers;
    bool shutdown = false;
};

}
