#pragma once

#include <stdint.h>

#include <string_view>

#include "hoytech/error.h"


namespace hoytech {

struct Parser {
  private:
    std::string_view s;

  public:
    Parser(std::string_view s) : s(s) {
    }

    bool isEof() const {
        return s.size() == 0;
    }

    uint8_t getByte() {
        if (s.size() < 1) throw herr("parse ends prematurely");
        uint8_t output = s[0];
        s = s.substr(1);
        return output;
    }

    std::string_view getBytes(size_t n) {
        if (s.size() < n) throw herr("parse ends prematurely");
        auto res = s.substr(0, n);
        s = s.substr(n);
        return res;
    };

    uint64_t getVarInt() {
        uint64_t res = 0;

        while (1) {
            if (s.size() == 0) throw herr("premature end of varint");
            uint64_t byte = s[0];
            s = s.substr(1);
            res = (res << 7) | (byte & 0b0111'1111);
            if ((byte & 0b1000'0000) == 0) break;
        }

        return res;
    }
};

struct Encoder {
  private:
    std::string s;

  public:
    std::string finish() {
        return std::move(s);
    }

    void putByte(uint8_t b) {
        s += (unsigned char)b;
    }

    void putBytes(std::string_view b) {
        s.append(b);
    }

    void putVarInt(uint64_t n) {
        if (n == 0) {
            s += '\0';
            return;
        }

        std::string o;

        while (n) {
            o.push_back(static_cast<unsigned char>(n & 0x7F));
            n >>= 7;
        }

        std::reverse(o.begin(), o.end());

        for (size_t i = 0; i < o.size() - 1; i++) {
            o[i] |= 0x80;
        }

        s += o;
    }
};

}
