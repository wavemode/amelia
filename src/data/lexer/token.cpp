#include "token.h"

#include "prelude.h"

namespace amelia {

bool Token::operator==(const Token &other) const {
  return type == other.type && location == other.location && contents == other.contents;
}

bool Token::operator!=(const Token &other) const { return !(*this == other); }

void Token::to_string(AbstractString &out) const {
  token_type_to_string(type, out);
  out.append("(");
  out.append(contents);
  out.append(")");
}

} // namespace amelia
