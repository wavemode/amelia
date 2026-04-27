#include "NumberReadError.h"

namespace amelia {

NumberReadError::NumberReadError() noexcept : message() {}

NumberReadError::NumberReadError(String message) noexcept : message(std::move(message)) {}

const char *NumberReadError::what() const noexcept { return message.c_str(); }

} // namespace amelia
