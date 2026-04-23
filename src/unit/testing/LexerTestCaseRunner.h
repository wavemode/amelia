#pragma once

#include "interface/testing/ITestCaseRunner.h"

namespace amelia {

class LexerTestCaseRunner : public ITestCaseRunner {
public:
  virtual void run_test_case(IString &output, CompilerTestCase input) override;
};

} // namespace amelia
