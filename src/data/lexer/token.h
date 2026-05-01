#pragma once

#include "data/lexer/token_type.h"
#include "data/source/location.h"
#include "data/text/text.h"

namespace amelia {

class IString;

struct Token {
  TokenType type;
  Location location;
  Text contents;

  bool operator==(const Token &other) const;

  bool operator!=(const Token &other) const;

  void to_string(IString &out) const;
};

} // namespace amelia
