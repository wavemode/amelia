#pragma once

#include "source/data/source_location_error.hpp"

namespace amelia {

struct Location;
class String;

/**
 * @class TypeError
 * @brief Exception thrown by the typechecker when it encounters an error.
 */
struct TypeError : public SourceLocationError {
  TypeError(Location loc, String message) noexcept;
};

} // namespace amelia
