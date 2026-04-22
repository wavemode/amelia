#pragma once

#include <stdexcept>

#include "data/core/String.h"

namespace amelia {

/**
 * @class CompilerTestCaseError
 * @brief An error that occurred while processing a compiler test case
 */
struct CompilerTestCaseError : public std::exception {
  const String message;

  CompilerTestCaseError() noexcept;
  CompilerTestCaseError(String message) noexcept;
  const char *what() const noexcept override;
};

} // namespace amelia
