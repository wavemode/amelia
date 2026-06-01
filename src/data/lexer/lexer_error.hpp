#pragma once

#include "data/source/location.hpp"
#include "data/source/source_location_error.hpp"
#include "data/util/string.hpp"

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
