#pragma once

#include "data/source/location.h"
#include "data/util/string.h"
#include "source_location_error.h"

namespace amelia {

/**
 * @class LexerError
 * @brief Exception thrown by the lexer when it encounters an error.
 */
struct LexerError : public SourceLocationError {
  LexerError(Location loc, String message) noexcept;
};

} // namespace amelia
