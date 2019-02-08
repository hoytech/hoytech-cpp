#pragma once

#include <assert.h>

#include <string_view>


namespace hoytech {

static inline bool assert_zerocopy_impl(std::string_view a, std::string_view b, bool want_zerocopy) {
    if (a.size() == 0 || b.size() == 0) return true;

    if (a.data() >= b.data() && a.data() < (b.data() + b.size())) return want_zerocopy;
    if (b.data() >= a.data() && b.data() < (a.data() + a.size())) return want_zerocopy;
    return !want_zerocopy;
}

}


#define assert_zerocopy(a,b) assert(hoytech::assert_zerocopy_impl((a), (b), true))
#define assert_copied(a,b) assert(hoytech::assert_zerocopy_impl((a), (b), false))
