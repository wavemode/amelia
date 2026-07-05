#pragma once

#include "source/data/location.hpp"
#include "source/data/source_location_error.hpp"
#include "util/data/string.hpp"

namespace amelia {

/**
 * @class ParserError
 * @brief Exception thrown by the parser when it encounters an error.
 */
struct ParserError : public SourceLocationError {
  ParserError(Location loc, String message) noexcept : SourceLocationError(loc, move(message)) {}
};

} // namespace amelia
