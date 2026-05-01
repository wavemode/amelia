#pragma once

#include <stdexcept>

#include "data/core/string.h"

namespace amelia {

/**
 * @class RuntimeError
 * @brief An unexpected error, usually indicating a bug in the program.
 */
struct RuntimeError : public std::exception {
  const String message;

  RuntimeError() noexcept;
  RuntimeError(String) noexcept;
  const char *what() const noexcept override;
};

} // namespace amelia
