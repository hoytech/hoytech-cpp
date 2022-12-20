#pragma once

#include <assert.h>

#include <string_view>


namespace hoytech {

inline bool assert_zerocopy_impl(std::string_view a, std::string_view b, bool want_zerocopy) {
    if (a.size() == 0 || b.size() == 0) return true;

    if (a.data() >= b.data() && a.data() < (b.data() + b.size())) return want_zerocopy;
    if (b.data() >= a.data() && b.data() < (a.data() + a.size())) return want_zerocopy;
    return !want_zerocopy;
}

inline bool is_zerocopy(std::string_view a, std::string_view b) {
    return assert_zerocopy_impl(a, b, true);
}

inline bool is_zerocopy_substr(std::string_view container, std::string_view substr) {
    if (substr.size() == 0) return true;
    return substr.data() >= container.data() && substr.data() + substr.size() <= container.data() + container.size();
}

}


#define assert_zerocopy(a,b) assert(hoytech::assert_zerocopy_impl((a), (b), true))
#define assert_copied(a,b) assert(hoytech::assert_zerocopy_impl((a), (b), false))
#define assert_zerocopy_substr(container,substr) assert(hoytech::is_zerocopy_substr((container), (substr)))
