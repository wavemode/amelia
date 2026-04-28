#include "Token.h"

#include "Prelude.h"

namespace amelia {

bool Token::operator==(const Token &other) const {
  return type == other.type && location == other.location && contents == other.contents;
}

bool Token::operator!=(const Token &other) const { return !(*this == other); }

void Token::to_string(IString &out) const {
  token_type_to_string(type, out);
  out.append("(");
  out.append(contents);
  out.append(")");
}

} // namespace amelia
