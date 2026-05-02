#include "lexer_error.h"
#include "prelude.h"

#include "data/util/text_utils.h"

namespace amelia {

LexerError::LexerError(Location loc, String message) noexcept
    : SourceLocationError(loc, std::move(message)) {}

} // namespace amelia
