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
    void run() {
        t = std::thread([this]() {
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

                auto now = std::chrono::steady_clock::now();

                while (!queue.empty()) {
                    auto &it = queue.top();
                    if (it.trigger > now) break;

                    if (live_timers.count(it.tok)) {
                        bool again;

                        {
                            lock.unlock(); // in case the callback wants to add/cancel other timers
                            again = it.cb();
                            lock.lock();
                        }

                        if (again) {
                            auto trigger = now + std::chrono::microseconds(it.interval);
                            queue.emplace(it.interval, it.tok, it.cb, trigger);
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

    cancel_token repeat_maybe(uint64_t interval_microseconds, std::function<bool()> cb) {
        std::unique_lock<std::mutex> lock(m);

        auto tok = next_cancel_token++;

        auto trigger = std::chrono::steady_clock::now() + std::chrono::microseconds(interval_microseconds);

        queue.emplace(interval_microseconds, tok, cb, trigger);
        live_timers.insert(tok);

        lock.unlock();
        cv.notify_all();

        return tok;
    }

    bool cancel(cancel_token tok) {
        std::unique_lock<std::mutex> lock(m);
        return live_timers.erase(tok); // returns true if timer was cancelled
    }

    cancel_token once(uint64_t interval_microseconds, const std::function<void()> &cb) {
        return repeat_maybe(interval_microseconds, [&, cb]{
            cb();
            return false;
        });
    }

    cancel_token repeat(uint64_t interval_microseconds, const std::function<void()> &cb) {
        return repeat_maybe(interval_microseconds, [&, cb]{
            cb();
            return true;
        });
    }

  private:
    struct item {
        item(uint64_t interval_, cancel_token tok_, std::function<bool()> cb_, std::chrono::steady_clock::time_point trigger_)
            : interval(interval_), tok(tok_), cb(cb_), trigger(trigger_) {}

        bool operator<(const item& rhs) const {
            return trigger > rhs.trigger;
        }

        uint64_t interval;
        cancel_token tok;
        std::function<bool()> cb;
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
