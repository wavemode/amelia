#pragma once

#include "source/data/source_location_error.hpp"

namespace amelia {

struct Location;
class String;

/**
 * @class ParserError
 * @brief Exception thrown by the parser when it encounters an error.
 */
struct ParserError : public SourceLocationError {
  ParserError(Location loc, String message) noexcept;
};

} // namespace amelia
