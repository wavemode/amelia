#include "LexerError.h"

namespace amelia {

LexerError::LexerError(Location loc, String message) noexcept : loc(loc), message(message) {}

const char *LexerError::what() const noexcept { return message.c_str(); }

} // namespace amelia