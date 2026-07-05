#pragma once

#include "prelude.hpp"

#include "source/data/location.hpp"

namespace amelia {

/**
 * @class SourceLocationError
 * @brief Exception associated with a specific location in source code input.
 */
struct SourceLocationError : public std::exception {
  const String message;

  SourceLocationError(Location loc, String message);
  const char *what() const noexcept override;
};

} // namespace amelia
