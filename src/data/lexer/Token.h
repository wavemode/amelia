#pragma once

#include <cstdint>

#include "data/lexer/Location.h"
#include "data/lexer/TokenType.h"
#include "util/slice/Slice.h"
#include "util/text/Text.h"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;
};

} // namespace amelia
