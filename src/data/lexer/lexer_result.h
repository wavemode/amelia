#pragma once

#include <cstddef>

#include "data/source/number_literal.h"
#include "data/source/string_literal.h"
#include "data/util/list.h"
#include "data/util/map.h"
#include "data/util/option.h"
#include "data/util/ref.h"
#include "data/util/slice.h"
#include "data/util/string.h"
#include "data/util/text.h"
#include "token.h"

namespace amelia {

struct LexerResult {
  List<Token> tokens;
  List<char> string_literal_buffer;
  Map<size_t, StringLiteral> string_literals;
  Map<size_t, NumberLiteral> number_literals;

  Option<StringLiteral> string_literal(size_t token_id) const noexcept {
    return string_literals.find(token_id);
  }

  Option<NumberLiteral> number_literal(size_t token_id) const noexcept {
    return number_literals.find(token_id);
  }

  void token_to_string(AbstractString &out, size_t token_id) {
    auto token = tokens[token_id];
    token_type_to_string(out, token.type);
    out.append("(");
    if (token.type == TokenType::STRING_LITERAL) {
      auto lit = string_literal(token_id).value();
      out.append('\"');
      out.append(Text(Slice(string_literal_buffer.data().ptr() + lit.buffer_offset, lit.length)));
      out.append('\"');
    } else if (token.type == TokenType::NUMBER) {
      auto lit = number_literal(token_id).value();
      out.append(lit.base_prefix);
      out.append(lit.integer_digits);
      if (lit.has_decimal_point) {
        out.append('.');
      }
      out.append(lit.fractional_digits);
      out.append(lit.exponent_prefix);
      out.append(lit.exponent_sign);
      out.append(lit.exponent_digits);
    } else {
      out.append(token.contents);
    }
    out.append(")");
  }
};

} // namespace amelia
