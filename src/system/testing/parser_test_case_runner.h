#pragma once

#include "interface/testing/test_case_runner.h"

namespace amelia {

class ParserTestCaseRunner : public ITestCaseRunner {
public:
  void run_test_case(AbstractString &output, CompilerTestCase test_case) override;
};

} // namespace amelia
