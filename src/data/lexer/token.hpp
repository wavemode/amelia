#pragma once

#include "data/lexer/token_type.hpp"
#include "data/source/location.hpp"
#include "data/util/text.hpp"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;
};

Text identifier_name(const Token &name);

} // namespace amelia
