#include "LexerTestCaseRunner.h"

#include "action/testing/compiler_test_cases.h"

namespace amelia {

void LexerTestCaseRunner::run_test_case(IString &output, CompilerTestCase test_case) {
  run_lexer_test_case(output, test_case);
}

} // namespace amelia
