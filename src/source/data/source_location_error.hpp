#pragma once

#include "source/data/location.hpp"

namespace amelia {

class String;

/**
 * @class SourceLocationError
 * @brief Exception associated with a specific location in source code input.
 */
struct SourceLocationError : public RuntimeError {
  SourceLocationError(const Location &loc, String &&message);

  const Location location;
};

} // namespace amelia
