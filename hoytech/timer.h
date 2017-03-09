#pragma once

/*

Timer thread implementation for C++11

(C) 2016 Doug Hoyte
2-clause BSD license

*/


#include <queue>
#include <condition_variable>
#include <thread>
#include <functional>
#include <set>


namespace hoytech {

using timer_cancel_token = uint64_t;

class timer {
  public:
    timer();
    ~timer();
    timer_cancel_token once(uint64_t interval_microseconds, std::function<void()> cb);
    timer_cancel_token repeat(uint64_t interval_microseconds, std::function<void()> cb);
    timer_cancel_token repeat_maybe(uint64_t interval_microseconds, std::function<bool()> cb);
    bool cancel(timer_cancel_token tok); // returns true if timer was cancelled

  private:
    struct item {
        item(uint64_t interval_, timer_cancel_token cancel_token_, std::function<bool()> cb_, std::chrono::steady_clock::time_point trigger_)
            : interval(interval_), cancel_token(cancel_token_), cb(cb_), trigger(trigger_) {}

        bool operator<(const item& rhs) const {
            return trigger > rhs.trigger;
        }

        uint64_t interval;
        timer_cancel_token cancel_token;
        std::function<bool()> cb;
        std::chrono::steady_clock::time_point trigger;
    };

    std::thread t;
    std::mutex m;
    std::condition_variable cv;
    timer_cancel_token next_cancel_token = 1;
    std::priority_queue<item> queue;
    std::set<timer_cancel_token> live_timers;
    bool shutdown = false;

    // Caller must have lock on mutex!
    void add_item(uint64_t interval_microseconds, timer_cancel_token tok, const std::function<bool()> &cb, std::chrono::steady_clock::time_point &trigger) {
        queue.emplace(interval_microseconds, tok, cb, trigger);
        live_timers.insert(tok);
    }
};

}
