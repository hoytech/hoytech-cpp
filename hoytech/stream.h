#pragma once

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <string_view>
#include <limits>
#include <optional>

#include "hoytech/time.h"
#include "hoytech/error.h"



namespace hoytech {

inline void _setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) throw hoytech::error("fcntl: couldn't get: ", strerror(errno));
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) throw hoytech::error("fcntl: couldn't set descriptor non-blocking: ", strerror(errno));
}

class StreamWriter {
  private:
    int fd;

  public:
    StreamWriter(int fd) : fd(fd) {
        _setNonBlocking(fd);
    }

    ~StreamWriter() {
        if (fd != -1) close(fd);
        fd = -1;
    }

    void write(std::string_view buf, uint64_t timeoutMilliseconds = std::numeric_limits<uint64_t>::max()) {
        bool hasTimeout = timeoutMilliseconds != std::numeric_limits<uint64_t>::max();
        uint64_t deadline = hasTimeout ? hoytech::curr_time_ms() + timeoutMilliseconds : 0;

        while (buf.size() > 0) {
            int timeoutMs = -1;

            if (hasTimeout) {
                uint64_t now = hoytech::curr_time_ms();
                if (hasTimeout && now >= deadline) throw hoytech::error("timed out");
                timeoutMs = (int)(deadline - now);
            }

            {
                struct pollfd pollData{fd, POLLOUT, 0};
                int ret = ::poll(&pollData, 1, timeoutMs);
                if (ret < 0) {
                    if (errno == EINTR) continue;
                    throw hoytech::error("poll failure: ", strerror(errno));
                }
                if (hasTimeout && ret == 0) throw hoytech::error("timed out");
            }

            ssize_t ret = ::write(fd, buf.data(), buf.size());
            if (ret < 0) {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                throw hoytech::error("write failure: ", strerror(errno));
            }

            buf = buf.substr(ret);
        }
    }
};


class StreamReader {
  private:
    int fd;
    std::string buf;

    char sep = '\n';
    std::optional<size_t> maxRecordSize;

  public:
    StreamReader(int fd) : fd(fd) {
        _setNonBlocking(fd);
    }

    ~StreamReader() {
        if (fd != -1) close(fd);
        fd = -1;
    }

    void setSep(char _sep) {
        sep = _sep;
    }

    void setMaxRecordSize(size_t _maxRecordSize) {
        maxRecordSize = _maxRecordSize;
    }

    std::string read(uint64_t timeoutMilliseconds = std::numeric_limits<uint64_t>::max()) {
        bool hasTimeout = timeoutMilliseconds != std::numeric_limits<uint64_t>::max();
        uint64_t deadline = hasTimeout ? hoytech::curr_time_ms() + timeoutMilliseconds : 0;

        while (1) {
            if (buf.size()) {
                auto pos = buf.find(sep);
                if (pos != std::string::npos) {
                    std::string output = buf.substr(0, pos);
                    if (maxRecordSize && output.size() > *maxRecordSize) throw hoytech::error("maxRecordSize exceeded");

                    buf = buf.substr(pos + 1);
                    return output;
                }

                if (maxRecordSize && buf.size() > *maxRecordSize) throw hoytech::error("maxRecordSize exceeded");
            }

            int timeoutMs = -1;

            if (hasTimeout) {
                uint64_t now = hoytech::curr_time_ms();
                if (now >= deadline) throw hoytech::error("timed out");
                timeoutMs = (int)(deadline - now);
            }

            {
                struct pollfd pollData{fd, POLLIN, 0};
                int ret = ::poll(&pollData, 1, timeoutMs);
                if (ret < 0) {
                    if (errno == EINTR) continue;
                    throw hoytech::error("poll failure: ", strerror(errno));
                }
                if (hasTimeout && ret == 0) throw hoytech::error("timed out");
            }

            char tmp[8192];
            ssize_t ret = ::read(fd, tmp, sizeof(tmp));
            if (ret == 0) throw hoytech::error("read failure: pipe closed");
            if (ret < 0) {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                throw hoytech::error("read failure: ", strerror(errno));
            }

            buf.append(tmp, tmp + ret);
        }
    }
};

}
