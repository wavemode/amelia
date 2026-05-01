#pragma once

#include <stdexcept>

#include "data/text/string.h"

namespace amelia {

/**
 * @class NumberLiteralReadError
 * @brief
 */
struct NumberLiteralReadError : public std::exception {
  const String message;

  NumberLiteralReadError() noexcept;
  NumberLiteralReadError(String) noexcept;
  const char *what() const noexcept override;
};

} // namespace amelia
