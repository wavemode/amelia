#include "token.hpp"

#include "data/util/text_utils.hpp"

#include "data/lexer/lexer.hpp"
#include "data/source/source_location_error.hpp"

namespace amelia {

String identifier_text(const Token &token, bool escaped) {
  switch (token.type) {
  case TokenType::IDENTIFIER:
  case TokenType::IDENTIFIER_NO_W:
  case TokenType::KEYWORD_THIS_TYPE:
  case TokenType::KEYWORD_SUPER:
    return String(token.contents);
  case TokenType::QUOTED_IDENTIFIER:
  case TokenType::QUOTED_IDENTIFIER_NO_W: {
    String result;
    Lexer::read_quoted_ident(result, token.contents, escaped);
    return result;
  }
  default:
    throw SourceLocationError(token.location, "Expected identifier");
  }
}

} // namespace amelia
