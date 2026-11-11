#pragma once

#include "lexer/data/token_type.hpp"
#include "source/data/location.hpp"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;
};

} // namespace amelia
