#include "lexer_error.hpp"

#include "source/data/location.hpp"

namespace amelia {

LexerError::LexerError(Location loc, String message) noexcept
    : SourceLocationError(loc, move(message)) {}

} // namespace amelia
