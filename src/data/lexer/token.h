#pragma once

#include "data/lexer/token_type.h"
#include "data/source/location.h"
#include "data/util/text.h"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;
};

} // namespace amelia
