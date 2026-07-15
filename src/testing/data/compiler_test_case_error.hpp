#pragma once

#include "util/data/string.hpp"

namespace amelia {

/**
 * @class CompilerTestCaseError
 * @brief An error that occurred while processing a compiler test case
 */
struct CompilerTestCaseError : public std::exception {
  const String message;

  CompilerTestCaseError() noexcept = default;
  CompilerTestCaseError(String message) noexcept : message(move(message)) {}
  const char *what() const noexcept override {
    return message.c_str();
  }
};

} // namespace amelia
