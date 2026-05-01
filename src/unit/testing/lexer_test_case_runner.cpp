#include "lexer_test_case_runner.h"

#include "action/testing/compiler_test_cases.h"
#include "data/testing/compiler_test_case.h"

namespace amelia {

void LexerTestCaseRunner::run_test_case(IString &output, CompilerTestCase test_case) {
  run_lexer_test_case(output, test_case);
}

} // namespace amelia
