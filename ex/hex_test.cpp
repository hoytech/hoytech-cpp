#include <iostream>
#include <string_view>

#include "hex.h"
#include "error.h"

int main() {
    if (hoytech::to_hex("\x01\x02\x03\x04") != "01020304") throw hoytech::error("fail");
    if (hoytech::to_hex("\x01\x02\x03\x04", true) != "0x01020304") throw hoytech::error("fail");

    if (hoytech::from_hex("01020304") != "\x01\x02\x03\x04") throw hoytech::error("fail");
    if (hoytech::from_hex("0x01020304") != "\x01\x02\x03\x04") throw hoytech::error("fail");

    if (hoytech::from_hex("1020304") != "\x01\x02\x03\x04") throw hoytech::error("fail");
    if (hoytech::from_hex("001020304") != hoytech::from_hex("0001020304")) throw hoytech::error("fail");

    if (hoytech::from_hex("") != "") throw hoytech::error("fail");
    if (hoytech::to_hex("") != "") throw hoytech::error("fail");
    if (hoytech::to_hex("", true) != "0x") throw hoytech::error("fail");

    std::cout << "ALL OK" << std::endl;
    return 0;
}
