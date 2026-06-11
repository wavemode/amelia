#include "token.hpp"

#include "data/util/text_utils.hpp"

#include "data/source/source_location_error.hpp"

namespace amelia {

Text identifier_text(const Token &name) {
  switch (name.type) {
  case TokenType::IDENTIFIER:
  case TokenType::IDENTIFIER_NO_W:
    return name.contents;
  case TokenType::QUOTED_IDENTIFIER:
  case TokenType::QUOTED_IDENTIFIER_NO_W:
    return TextUtils::substr_bytes(name.contents, 1, name.contents.size() - 1);
  default:
    throw SourceLocationError(name.location, "Expected identifier");
  }
}

} // namespace amelia
