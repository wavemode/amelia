#pragma once

#include "interface/testing/ITestCaseRunner.h"

namespace amelia {

class LexerTestCaseRunner : public ITestCaseRunner {
public:
  virtual void run_test_case(IString &output, CompilerTestCase test_case) override;
};

} // namespace amelia
