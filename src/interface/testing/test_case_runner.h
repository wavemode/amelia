#pragma once

namespace amelia {

class IString;
class CompilerTestCase;

class ITestCaseRunner {
public:
  virtual void run_test_case(IString &output, CompilerTestCase input) = 0;
};

} // namespace amelia
