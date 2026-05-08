#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.h"
#include "data/lexer/token.h"
#include "data/source/number_literal.h"
#include "data/util/text_utils.h"
#include "prelude.h"

namespace amelia {

class TokenFormatter {
public:
  TokenFormatter(const AbstractTokenRepository &repo) : m_repo(repo) {}

  void format_token(AbstractString &out, size_t token_id) const {
    auto token = m_repo.get_token(token_id);
    token_type_to_string(out, token.type);
    out.append("(");
    if (token.type == TokenType::STRING_LITERAL) {
      out.append('\"');
      for (uint32_t cp : m_repo.string_literal_contents(token_id)) {
        switch (cp) {
        case '\\':
          out.append('\\');
          out.append('\\');
          break;
        case '\a':
          out.append('\\');
          out.append('a');
          break;
        case '\b':
          out.append('\\');
          out.append('b');
          break;
        case '\f':
          out.append('\\');
          out.append('f');
          break;
        case '\n':
          out.append('\\');
          out.append('n');
          break;
        case '\r':
          out.append('\\');
          out.append('r');
          break;
        case '\t':
          out.append('\\');
          out.append('t');
          break;
        case '\v':
          out.append('\\');
          out.append('v');
          break;
        case '\'':
          out.append('\\');
          out.append('\'');
          break;
        case '"':
          out.append('\\');
          out.append('\"');
          break;
        default:
          out.append(cp);
        }
      }
      out.append('\"');
    } else if (token.type == TokenType::NUMBER || token.type == TokenType::NUMBER_FIELD) {
      auto lit = m_repo.get_number_literal(token_id);
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

private:
  const AbstractTokenRepository &m_repo;
};

} // namespace amelia
