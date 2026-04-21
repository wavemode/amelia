#pragma once

namespace amelia {

class IString;
class CompilerTestCase;

class ITestCaseRunner {
public:
  virtual void run_test_case(CompilerTestCase input, IString &output) = 0;
};

} // namespace amelia
