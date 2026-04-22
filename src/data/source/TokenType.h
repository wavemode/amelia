#pragma once

#include <stdexcept>

#include "data/core/Slice.h"
#include "interface/text/IString.h"

namespace amelia {

enum class TokenType {
  IDENTIFIER,
  INTEGER,
  FLOAT,

  ASSIGN,
  EQUAL,

  END_OF_FILE,
};

inline void token_type_to_string(TokenType type, IString &out) {
  switch (type) {
  case TokenType::IDENTIFIER:
    out.append("IDENTIFIER");
    break;
  case TokenType::INTEGER:
    out.append("INTEGER");
    break;
  case TokenType::FLOAT:
    out.append("FLOAT");
    break;
  case TokenType::ASSIGN:
    out.append("ASSIGN");
    break;
  case TokenType::EQUAL:
    out.append("EQUAL");
    break;
  case TokenType::END_OF_FILE:
    out.append("END_OF_FILE");
    break;
  default:
    throw std::invalid_argument("Invalid TokenType");
  }
}

} // namespace amelia
