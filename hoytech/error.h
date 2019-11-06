#pragma once

#include <stdexcept>
#include <sstream>
#include <string>

namespace hoytech {

inline void build_string(std::ostream&) { }

template<class First, class... Rest>
inline void build_string(std::ostream& o, const First& value, const Rest&... rest) {
    o << value;
    build_string(o, rest...);
}

template<class... T>
std::string concat_string(const T&... value) {
    std::ostringstream o;
    build_string(o, value...);
    return o.str();
}

template<class... T>
std::runtime_error error(const T&... value) {
    std::ostringstream o;
    build_string(o, value...);
    return std::runtime_error(o.str());
}

}
