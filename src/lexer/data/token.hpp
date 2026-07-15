#pragma once

#include "lexer/data/token_type.hpp"
#include "source/data/location.hpp"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;
};

String identifier_text(const Token &name, bool quoted = true, bool escaped = false);

} // namespace amelia
