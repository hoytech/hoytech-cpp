#include <chrono>
#include <iostream> //FIXME

#include "timer.h"


namespace hoytech {

timer::timer() {
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

                if (live_timers.erase(it.cancel_token)) {
                    bool again;

                    {
                        lock.unlock(); // in case the callback wants to add/cancel other timers
                        again = it.cb();
                        lock.lock();
                    }

                    if (again) {
                        auto trigger = now + std::chrono::microseconds(it.interval);
                        add_item(it.interval, it.cancel_token, it.cb, trigger);
                    }
                }

                queue.pop();
            }
        }
    });
}

timer::~timer() {
    std::unique_lock<std::mutex> lock(m);
    shutdown = true;

    lock.unlock();
    cv.notify_all();

    t.join();
}

timer_cancel_token timer::once(uint64_t interval_microseconds, std::function<void()> cb) {
    return repeat_maybe(interval_microseconds, [&, cb]{
        cb();
        return false;
    });
}

timer_cancel_token timer::repeat(uint64_t interval_microseconds, std::function<void()> cb) {
    return repeat_maybe(interval_microseconds, [&, cb]{
        cb();
        return true;
    });
}

timer_cancel_token timer::repeat_maybe(uint64_t interval_microseconds, std::function<bool()> cb) {
    std::unique_lock<std::mutex> lock(m);

    timer_cancel_token tok = next_cancel_token++;

    auto trigger = std::chrono::steady_clock::now() + std::chrono::microseconds(interval_microseconds);

    add_item(interval_microseconds, tok, cb, trigger);

    lock.unlock();
    cv.notify_all();

    return tok;
}

bool timer::cancel(timer_cancel_token tok) {
    std::unique_lock<std::mutex> lock(m);
    return live_timers.erase(tok);
}


}
