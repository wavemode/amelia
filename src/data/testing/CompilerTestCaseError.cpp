#include "CompilerTestCaseError.h"
#include "data/core/String.h"

namespace amelia {

CompilerTestCaseError::CompilerTestCaseError() noexcept {}

CompilerTestCaseError::CompilerTestCaseError(String message) noexcept : message(message) {}

const char *CompilerTestCaseError::what() const noexcept { return message.c_str(); }

} // namespace amelia
