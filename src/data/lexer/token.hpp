#pragma once

#include "prelude.hpp"

#include "data/lexer/token_type.hpp"
#include "data/source/location.hpp"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;
};

String identifier_text(const Token &name, bool quoted = true, bool escaped = false);

} // namespace amelia
