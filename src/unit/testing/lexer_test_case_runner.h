#pragma once

#include "interface/testing/test_case_runner.h"

namespace amelia {

class LexerTestCaseRunner : public ITestCaseRunner {
public:
  virtual void run_test_case(AbstractString &output, CompilerTestCase test_case) override;
};

} // namespace amelia
