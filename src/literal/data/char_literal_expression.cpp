#include "char_literal_expression.hpp"

#include "source/data/char_literal.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize CharLiteralExpression::serialize() const {
  String repr;
  repr.append("CharLiteralExpression[");
  serialize_char_literal(value).to_string(repr);
  repr.append("]");
  return Serialize::literal(move(repr));
}

} // namespace amelia
