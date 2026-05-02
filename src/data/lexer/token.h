#pragma once

#include "data/lexer/token_type.h"
#include "data/source/location.h"
#include "data/util/text.h"

namespace amelia {

class AbstractString;
class LexerResult;

struct Token {
  TokenType type;
  Location location;
  Text contents;

  bool operator==(const Token &other) const;

  bool operator!=(const Token &other) const;

  static void to_string(AbstractString &out, size_t token_id, const LexerResult &lr);
};

} // namespace amelia
