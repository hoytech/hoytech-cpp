#pragma once

#include <queue>
#include <condition_variable>
#include <thread>
#include <functional>
#include <set>


namespace hoytech {

class timer {
  public:
    void run();
    ~timer();

    using cancel_token = uint64_t;

    cancel_token repeat_maybe(uint64_t interval_microseconds, std::function<bool()> cb);
    bool cancel(cancel_token tok); // returns true if timer was cancelled

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
