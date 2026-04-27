#pragma once

#include <stdexcept>

namespace amelia {

class IString;

enum class TokenType {
  KEYWORD,

  IDENTIFIER,
  NUMBER,

  ASSIGN,
  EQUAL,

  END_OF_FILE,
};

void token_type_to_string(TokenType type, IString &out);

} // namespace amelia
