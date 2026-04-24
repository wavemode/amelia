#pragma once

#include <stdexcept>

namespace amelia {

class IString;

enum class TokenType {
  IDENTIFIER,
  INTEGER,
  FLOAT,

  ASSIGN,
  EQUAL,

  END_OF_FILE,
};

void token_type_to_string(TokenType type, IString &out);

} // namespace amelia
