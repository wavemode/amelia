#include "parser_test_case_runner.hpp"

#include "action/testing/compiler_test_cases.hpp"
#include "data/testing/compiler_test_case.hpp"

namespace amelia {

void ParserTestCaseRunner::run_test_case(AbstractString &output, CompilerTestCase test_case) {
  run_parser_test_case(output, test_case);
}

} // namespace amelia
