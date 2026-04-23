#pragma once

#include "interface/testing/ITestCaseRunner.h"

namespace amelia {

class LexerTestCaseRunner : public ITestCaseRunner {
public:
  virtual void run_test_case(CompilerTestCase input, IString &output) override;
};

} // namespace amelia
