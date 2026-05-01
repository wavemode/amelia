#pragma once

#include <stdexcept>

#include "data/core/string.h"

namespace amelia {

/**
 * @class StringLiteralReadError
 * @brief
 */
struct StringLiteralReadError : public std::exception {
  const String message;

  StringLiteralReadError() noexcept;
  StringLiteralReadError(String) noexcept;
  const char *what() const noexcept override;
};

} // namespace amelia
