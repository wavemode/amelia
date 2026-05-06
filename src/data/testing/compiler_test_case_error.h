#pragma once

#include <stdexcept>

#include "data/util/string.h"

namespace amelia {

/**
 * @class CompilerTestCaseError
 * @brief An error that occurred while processing a compiler test case
 */
struct CompilerTestCaseError : public std::exception {
  const String message;

  CompilerTestCaseError() noexcept = default;
  CompilerTestCaseError(String message) noexcept : message(std::move(message)) {}
  const char *what() const noexcept override {
    return message.c_str();
  }
};

} // namespace amelia
