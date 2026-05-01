#pragma once

namespace amelia {

class AbstractString;
class CompilerTestCase;

class ITestCaseRunner {
public:
  virtual void run_test_case(AbstractString &output, CompilerTestCase input) = 0;
};

} // namespace amelia
