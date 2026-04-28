#include "NumberLiteralReadError.h"

namespace amelia {

NumberLiteralReadError::NumberLiteralReadError() noexcept : message() {}

NumberLiteralReadError::NumberLiteralReadError(String message) noexcept
    : message(std::move(message)) {}

const char *NumberLiteralReadError::what() const noexcept { return message.c_str(); }

} // namespace amelia
