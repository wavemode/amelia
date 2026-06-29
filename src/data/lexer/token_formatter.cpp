#include "token_formatter.hpp"

#include "data/lexer/lexer.hpp"
#include "data/source/char_literal.hpp"
#include "data/source/identifier.hpp"
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
    serialize_char_literal(Lexer::read_char_literal(token.contents)).to_string(out);
    break;
  }
  case TokenType::IDENTIFIER:
  case TokenType::IDENTIFIER_NO_W:
    Identifier(token.contents).pretty_print(out);
    break;
  case TokenType::QUOTED_IDENTIFIER:
  case TokenType::QUOTED_IDENTIFIER_NO_W:
    Lexer::read_quoted_ident(token.contents).pretty_print(out);
    break;
  default:
    out.append(token.contents);
    break;
  }
  out.append(")");
}

} // namespace amelia
