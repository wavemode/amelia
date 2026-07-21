#include "token.hpp"

#include "util/data/text_utils.hpp"

#include "lexer/data/lexer.hpp"
#include "source/data/identifier.hpp"
#include "source/data/source_location_error.hpp"

namespace amelia {

String identifier_text(const Token &token, bool quoted, bool escaped) {
  String result;
  switch (token.type) {
  case TokenType::IDENTIFIER:
  case TokenType::IDENTIFIER_NO_W:
    Identifier(token.contents).pretty_print(result, quoted, escaped);
    break;
  case TokenType::QUOTED_IDENTIFIER:
  case TokenType::QUOTED_IDENTIFIER_NO_W:
    Lexer::read_quoted_ident(token.contents).pretty_print(result, quoted, escaped);
    break;
  default:
    throw SourceLocationError(token.location, "Expected identifier in identifier_text()");
  }
  return result;
}

} // namespace amelia
