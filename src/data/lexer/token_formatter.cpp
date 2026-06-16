#include "token_formatter.hpp"

#include "data/lexer/lexer.hpp"
#include "data/util/integer.hpp"

namespace amelia {

TokenFormatter::TokenFormatter(const AbstractTokenRepository &repo) : m_repo(repo) {}

void TokenFormatter::format_token(AbstractString &out, size_t token_id) const {
  auto token = m_repo.get_token(token_id);
  token_type_to_string(out, token.type);
  out.append("(");
  switch (token.type) {
  case TokenType::STRING_LITERAL:
    out.append('"');
    Lexer::read_string_literal(out, token.contents, true);
    out.append('"');
    break;
  case TokenType::CHAR_LITERAL: {
    out.append('\'');
    uint32_t code_point = Lexer::read_char_literal(token.contents);
    out.append("\\U");
    String hex_code_point;
    Integer(code_point).to_hex_string(hex_code_point);
    TextUtils::pad_left_into(out, hex_code_point, 8, "0");
    out.append('\'');
    break;
  }
  case TokenType::QUOTED_IDENTIFIER:
  case TokenType::QUOTED_IDENTIFIER_NO_W:
    out.append('`');
    out.append(identifier_text(token, true));
    out.append('`');
    break;
  default:
    out.append(token.contents);
    break;
  }
  out.append(")");
}

} // namespace amelia
