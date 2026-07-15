#include "parser_test_case_runner.hpp"

#include "testing/data/compiler_test_case.hpp"
#include "testing/logic/compiler_test_cases.hpp"

namespace amelia {

void ParserTestCaseRunner::run_test_case(AbstractString &output, CompilerTestCase test_case) {
  run_parser_test_case(output, test_case);
}

} // namespace amelia
