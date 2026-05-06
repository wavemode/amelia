#include "runtime_error.h"
#include "prelude.h"

namespace amelia {

RuntimeError::RuntimeError() noexcept : message() {}

RuntimeError::RuntimeError(String message) noexcept : message(std::move(message)) {}

const char *RuntimeError::what() const noexcept {
  return message.c_str();
}

} // namespace amelia
