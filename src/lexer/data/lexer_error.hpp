#pragma once

#include "source/data/source_location_error.hpp"

namespace amelia {

/**
 * @class LexerError
 * @brief Exception thrown by the lexer when it encounters an error.
 */
struct LexerError : public SourceLocationError {
  LexerError(Location loc, String message) noexcept;
};

} // namespace amelia
