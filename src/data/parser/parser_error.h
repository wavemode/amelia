#pragma once

#include "data/source/location.h"
#include "data/source/source_location_error.h"
#include "data/util/string.h"

namespace amelia {

/**
 * @class ParserError
 * @brief Exception thrown by the parser when it encounters an error.
 */
struct ParserError : public SourceLocationError {
  ParserError(Location loc, String message) noexcept
      : SourceLocationError(loc, std::move(message)) {}
};

} // namespace amelia
