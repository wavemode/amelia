#pragma once

#include "data/source/location.hpp"
#include "data/source/source_location_error.hpp"
#include "data/util/string.hpp"

namespace amelia {

/**
 * @class ParserError
 * @brief Exception thrown by the parser when it encounters an error.
 */
struct ParserError : public SourceLocationError {
  ParserError(Location loc, String message) noexcept : SourceLocationError(loc, move(message)) {}
};

} // namespace amelia
