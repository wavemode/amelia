#include "string_literal_read_error.h"

namespace amelia {

StringLiteralReadError::StringLiteralReadError() noexcept : message() {}

StringLiteralReadError::StringLiteralReadError(String message) noexcept
    : message(std::move(message)) {}

const char *StringLiteralReadError::what() const noexcept { return message.c_str(); }

} // namespace amelia
