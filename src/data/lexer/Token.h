#pragma once

#include <cstdint>

#include "data/lexer/Location.h"
#include "data/lexer/TokenType.h"
#include "util/slice/Slice.h"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Slice<uint32_t> contents;
};

} // namespace amelia
