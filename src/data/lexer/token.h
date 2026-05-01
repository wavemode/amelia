#pragma once

#include "data/core/text.h"
#include "data/lexer/token_type.h"
#include "data/source/location.h"

namespace amelia {

class AbstractString;

struct Token {
  TokenType type;
  Location location;
  Text contents;

  bool operator==(const Token &other) const;

  bool operator!=(const Token &other) const;

  void to_string(AbstractString &out) const;
};

} // namespace amelia
