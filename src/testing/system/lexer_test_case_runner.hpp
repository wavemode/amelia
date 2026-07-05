#pragma once

#include "testing/interface/test_case_runner.hpp"

namespace amelia {

class LexerTestCaseRunner : public ITestCaseRunner {
public:
  void run_test_case(AbstractString &output, CompilerTestCase test_case) override;
};

} // namespace amelia
