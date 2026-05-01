#include "compiler_test_case_error.h"
#include "prelude.h"

namespace amelia {

CompilerTestCaseError::CompilerTestCaseError() noexcept {}

CompilerTestCaseError::CompilerTestCaseError(String message) noexcept
    : message(std::move(message)) {}

const char *CompilerTestCaseError::what() const noexcept { return message.c_str(); }

} // namespace amelia
