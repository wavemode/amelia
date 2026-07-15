#include "string_literal_expression.hpp"

#include "source/data/char_literal.hpp"
#include "util/data/char_iterator.hpp"
#include "util/data/serialize.hpp"

namespace amelia {

Serialize StringLiteralExpression::serialize() const {
  String repr;
  repr.append("StringLiteralExpression[\"");
  for (uint32_t ch : value) {
    serialize_char_literal(ch, false).to_string(repr);
  }
  repr.append("\"]");
  return Serialize::literal(move(repr));
}

} // namespace amelia
