#pragma once

#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/eventfd.h>
#include <poll.h>

#include <string>
#include <thread>
#include <functional>

#include "hoytech/error.h"
#include "hoytech/time.h"



namespace hoytech {


class file_change_monitor {
  private:
    std::thread t;
    uint64_t debounce_us = 50 * 1000;
    int inotify_fd = -1;
    int inotify_wd = -1;
    int eventfd_fd = -1;
    bool shutdown = false;

    void cleanup() {
        if (eventfd_fd != -1) {
            ::close(eventfd_fd);
            eventfd_fd = -1;
        }

        if (inotify_wd != -1) {
            ::inotify_rm_watch(inotify_fd, inotify_wd);
            inotify_wd = -1;
        }

        if (inotify_fd != -1) {
            ::close(inotify_fd);
            inotify_fd = -1;
        }
    }

  public:
    file_change_monitor(std::string path) {
       inotify_fd = ::inotify_init1(IN_NONBLOCK);
        if (inotify_fd < 0) throw hoytech::error("unable to create inotify descriptor");

        inotify_wd = ::inotify_add_watch(inotify_fd, path.c_str(), IN_MODIFY);
        if (inotify_fd < 0) throw hoytech::error("unable to add watch to inotify descriptor");

        eventfd_fd = ::eventfd(0, 0);
        if (eventfd_fd < 0) throw hoytech::error("unable to create eventfd descriptor");
    }

    void setDebounce(uint64_t ms) {
        debounce_us = ms * 1000;
    }

    void run(std::function<void()> cb) {
        if (shutdown) throw hoytech::error("inotify watcher already shutdown");

        t = std::thread([cb, this]() {
            struct pollfd pollfd_array[2] = {
                { inotify_fd, POLLIN, 0 },
                { eventfd_fd, POLLIN, 0 }
            };

            uint64_t trigger_time = 0;

            while (1) {
                int timeout_ms = -1;

                if (trigger_time) {
                    uint64_t now = hoytech::curr_time_us();

                    if (now < trigger_time) {
                        timeout_ms = (trigger_time - now) / 1000;
                    } else {
                        timeout_ms = 0;
                    }

                    if (timeout_ms == 0) {
                        trigger_time = 0;
                        cb();
                        continue;
                    }
                }

                int rv = ::poll(pollfd_array, 2, timeout_ms);
                if (shutdown) return;

                if (rv == -1 && errno == EINTR) continue;
                if (rv == -1) return;
                if (rv == 0) continue;
                struct inotify_event event;
                rv = ::read(inotify_fd, (char*)&event, sizeof(event));
                if (shutdown) return;

                if (rv == -1 && (errno == EINTR || errno == EAGAIN)) continue;
                if (rv == -1) return;
                if (rv != sizeof(event)) return;

                if (trigger_time == 0) trigger_time = hoytech::curr_time_us() + debounce_us;
            }
        });
    }

    ~file_change_monitor() {
        shutdown = true;

        if (t.joinable()) {
            if (eventfd_fd != -1) {
                uint64_t event_msg = 1;
                int rv = write(eventfd_fd, &event_msg, sizeof(event_msg));
                (void)rv;
            }

            t.join();
        }

        cleanup();
    }
};


}
