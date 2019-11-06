#pragma once

#include <string>
#include <string_view>

#include "hoytech/error.h"

namespace hoytech {

std::string to_hex(std::string_view input, bool prefixed = false);
std::string from_hex(std::string_view input);

}
