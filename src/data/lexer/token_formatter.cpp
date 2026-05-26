#include "token_formatter.h"

#include "data/lexer/lexer.h"

namespace amelia {

TokenFormatter::TokenFormatter(const AbstractTokenRepository &repo) : m_repo(repo) {}

void TokenFormatter::format_token(AbstractString &out, size_t token_id) const {
  auto token = m_repo.get_token(token_id);
  token_type_to_string(out, token.type);
  out.append("(");
  switch (token.type) {
  case TokenType::STRING_LITERAL:
  case TokenType::MULTILINE_STRING_LITERAL:
  case TokenType::RAW_STRING_LITERAL:
  case TokenType::RAW_MULTILINE_STRING_LITERAL:
    out.append('"');
    Lexer::read_string_literal(out, token.contents, true);
    out.append('"');
    break;
  case TokenType::CHAR_LITERAL:
    out.append('\'');
    Lexer::read_char_literal(out, token.contents, true);
    out.append('\'');
    break;
  default:
    out.append(token.contents);
    break;
  }
  out.append(")");
}

} // namespace amelia
