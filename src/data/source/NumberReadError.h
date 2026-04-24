#pragma once

#include <stdexcept>

#include "data/text/String.h"

namespace amelia {

/**
 * @class NumberReadError
 * @brief
 */
struct NumberReadError : public std::exception {
  const String message;

  NumberReadError() noexcept;
  NumberReadError(String) noexcept;
  const char *what() const noexcept override;
};

} // namespace amelia
