#pragma once

#include "data/source/Location.h"
#include "data/source/TokenType.h"
#include "data/text/Text.h"

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
