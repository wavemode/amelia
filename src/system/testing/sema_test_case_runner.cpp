#include "sema_test_case_runner.hpp"

#include "action/testing/compiler_test_cases.hpp"
#include "data/testing/compiler_test_case.hpp"

namespace amelia {
SemaTestCaseRunner::SemaTestCaseRunner(IFileLoader &file_loader) : m_file_loader(file_loader) {}

void SemaTestCaseRunner::run_test_case(AbstractString &output, CompilerTestCase test_case) {
  run_sema_test_case(m_file_loader, output, test_case);
}

} // namespace amelia
