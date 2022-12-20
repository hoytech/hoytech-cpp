#pragma once

#include <stdio.h>

#include <string>
#include <string_view>

#include "hoytech/error.h"

namespace hoytech {

inline std::string to_hex(std::string_view input, bool prefixed = false) {
    size_t prefix_offset = (prefixed ? 2 : 0);

    std::string output(2*input.length() + prefix_offset, '\0');

    if (prefixed) {
        output[0] = '0';
        output[1] = 'x';
    }

    static const char *lookup = "0123456789abcdef";

    for (size_t i = 0; i < input.length(); i++) {
        output[i*2 + prefix_offset] = lookup[(input[i] >> 4) & 0x0f];
        output[i*2 + 1 + prefix_offset] = lookup[input[i] & 0x0f];
    }

    return output;
}

inline std::string to_hex(uint64_t input, bool prefixed = false) {
    char buf[32];

    snprintf(buf, sizeof(buf), "%s%lx", prefixed ? "0x" : "", input);

    return std::string(buf);
}

inline std::string from_hex(std::string_view input) {
    if (input.length() >= 2 && input.substr(0,2) == "0x") input = input.substr(2);

    std::string padded; // does a copy if given non-even number of hex digits
    if ((input.length() % 2) != 0) {
        padded += '0';
        padded += input;
        input = padded;
    }

    std::string output(input.length()/2, '\0');

    auto decode = [](unsigned char c){
        if ('0' <= c && c <= '9') return c - '0';
        else if ('a' <= c && c <= 'f') return c - 'a' + 10;
        else if ('A' <= c && c <= 'F') return c - 'A' + 10;
        else throw hoytech::error("unexpected character in from_hex: ", (int)c);
    };

    for(size_t i=0; i<input.length(); i+=2) {
        output[i/2] = (decode(input[i]) << 4) | decode(input[i+1]);
    }

    return output;
}

}
