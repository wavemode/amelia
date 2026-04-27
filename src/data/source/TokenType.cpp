#include "TokenType.h"

#include "Prelude.h"

void amelia::token_type_to_string(TokenType type, IString &out) {
  switch (type) {
  case TokenType::IDENTIFIER:
    out.append("IDENTIFIER");
    break;
  case TokenType::NUMBER:
    out.append("NUMBER");
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
    throw RuntimeError("Invalid TokenType");
  }
}
