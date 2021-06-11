#include <unistd.h>

#include <iostream>

#include "hoytech/timer.h"

int main() {
    hoytech::timer t;

    // Can install before run()
    auto cancel1 = t.repeat(500'000, []{
        std::cout << "Every .5 seconds" << std::endl;
    });

    t.repeat(0, [&]{
        std::cout << "0 INTERVAL = NEVER HAPPENS" << std::endl;
    });

    t.run();

    // Can also install after run()
    t.once(2*1'000'000, [&]{
        std::cout << "Cancelling .5 second repeat, starting 1 second repeat" << std::endl;

        if (t.cancel(cancel1)) std::cout << "OK cancel was removed" << std::endl;
        if (!t.cancel(cancel1)) std::cout << "OK cancel was NOT removed the second time" << std::endl;

        // Can even install from inside callback
        t.repeat_maybe(700'000, [count(0)]() mutable {
            std::cout << "Counting up to 5: " << ++count << std::endl;
            return count < 5;
        });
    });

    t.repeat_adjustable(1*1'000'000, [&t, curr(1)]() mutable {
        curr *= 2;
        std::cout << "CURR SLEEP: " << curr << std::endl;
        return curr * 1'000'000;
    });

    sleep(16);

    std::cout << "All done, let's destroy the timer" << std::endl;

    return 0;
}
