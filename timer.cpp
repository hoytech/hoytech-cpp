#include <chrono>

#include "hoytech/timer.h"


namespace hoytech {

void timer::run() {
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

timer::~timer() {
    if (!t.joinable()) return;

    std::unique_lock<std::mutex> lock(m);
    shutdown = true;

    lock.unlock();
    cv.notify_all();

    t.join();
}

hoytech::timer::cancel_token timer::repeat_maybe(uint64_t interval_microseconds, std::function<bool()> cb) {
    std::unique_lock<std::mutex> lock(m);

    auto tok = next_cancel_token++;

    auto trigger = std::chrono::steady_clock::now() + std::chrono::microseconds(interval_microseconds);

    queue.emplace(interval_microseconds, tok, cb, trigger);
    live_timers.insert(tok);

    lock.unlock();
    cv.notify_all();

    return tok;
}

bool timer::cancel(hoytech::timer::cancel_token tok) {
    std::unique_lock<std::mutex> lock(m);
    return live_timers.erase(tok);
}


}
