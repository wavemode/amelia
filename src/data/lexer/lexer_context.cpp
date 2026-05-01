#include "lexer_error.h"

#include "data/text/text_utils.h"

namespace amelia {

namespace {
String make_message(Location loc, String message) {
  message.append(" (at ");
  message.append(loc.filename);
  message.append(':');
  TextUtils::to_string(message, loc.line);
  message.append(':');
  TextUtils::to_string(message, loc.column);
  message.append(')');
  return message;
}
} // namespace

LexerError::LexerError(Location loc, String message) noexcept
    : message(make_message(loc, std::move(message))) {}

const char *LexerError::what() const noexcept { return message.c_str(); }

} // namespace amelia
