#pragma once

namespace amelia {

struct AbstractString;
struct CompilerTestCase;

struct ITestCaseRunner {
  virtual void run_test_case(AbstractString &output, CompilerTestCase input) = 0;
};

} // namespace amelia
