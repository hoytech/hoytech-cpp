#pragma once

#include <string>
#include <string_view>
#include <cstdint>


namespace hoytech {


// UTF-8 aware string truncation. Truncates the string in-place so that the total
// length (including ellipsis) fits in `len` bytes. It won't leave UTF-8 partials,
// but it doesn't understand grapheme clusters so it might cut in the middle of
// multi-codepoint sequences.

inline void truncateInPlace(std::string &str, size_t len, std::string_view ellipsis = "…") {
    if (str.size() <= len) return;

    if (ellipsis.size() > len) len = 0;
    else len = len - ellipsis.size();

    str.resize(len);

    static const uint8_t MASK = 0b1100'0000;
    static const uint8_t CONT = 0b1000'0000;

    if (str.size()) {
        uint8_t curr = str.back() & MASK;

        if (curr == MASK) {
            str.pop_back();
        } else if (curr == CONT) {
            do {
                str.pop_back();
            } while (str.size() && (str.back() & MASK) == CONT);

            if (str.size()) str.pop_back();
        }
    }

    str.append(ellipsis);
}


}
