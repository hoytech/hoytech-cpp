#pragma once

#include <sys/time.h>

namespace hoytech {


inline uint64_t curr_time_s() {
    return ::time(nullptr);
}

inline uint64_t curr_time_us() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}


}
