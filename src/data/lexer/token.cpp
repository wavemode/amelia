#include "token.h"

#include "data/lexer/lexer_result.h"
#include "prelude.h"

namespace amelia {

bool Token::operator==(const Token &other) const {
  return type == other.type && location == other.location && contents == other.contents;
}

bool Token::operator!=(const Token &other) const { return !(*this == other); }

void Token::to_string(AbstractString &out, size_t token_id, const LexerResult &lr) {
  auto token = lr.tokens()[token_id];
  token_type_to_string(out, token.type);
  out.append("(");
  if (token.type == TokenType::STRING_LITERAL) {
    out.append('"');
    out.append(lr.string_literal(token_id).value());
    out.append('"');
  } else if (token.type == TokenType::NUMBER) {
    auto number_literal = lr.number_literal(token_id).value();
    out.append("base_prefix=\"");
    out.append(number_literal.base_prefix);
    out.append("\", integer_digits=\"");
    out.append(number_literal.integer_digits);
    out.append("\", has_decimal_point=");
    if (number_literal.has_decimal_point) {
      out.append("true");
    } else {
      out.append("false");
    }
    out.append(", fractional_digits=\"");
    out.append(number_literal.fractional_digits);
    out.append("\", exponent_prefix=\"");
    out.append(number_literal.exponent_prefix);
    out.append("\", exponent_sign=\"");
    out.append(number_literal.exponent_sign);
    out.append("\", exponent_digits=\"");
    out.append(number_literal.exponent_digits);
    out.append("\"");
  } else {
    out.append(token.contents);
  }
  out.append(")");
}

} // namespace amelia
