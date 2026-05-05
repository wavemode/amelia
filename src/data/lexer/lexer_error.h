#pragma once

#include "data/source/location.h"
#include "data/source/source_location_error.h"
#include "data/util/string.h"

namespace amelia {

/**
 * @class LexerError
 * @brief Exception thrown by the lexer when it encounters an error.
 */
struct LexerError : public SourceLocationError {
  LexerError(Location loc, String message) noexcept
      : SourceLocationError(loc, std::move(message)) {}
};

} // namespace amelia
