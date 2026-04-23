#pragma once

#include "interface/core/IFileWriter.h"
#include "interface/testing/ITestCaseRunner.h"

namespace amelia {

class CompilerTestCaseExecutor {
public:
  CompilerTestCaseExecutor(ITestCaseRunner *test_case_runner, IFileWriter *file_writer);

  bool execute_test_case(const CompilerTestCase &test_case);
  bool update_expected_output(const CompilerTestCase &test_case);

private:
  ITestCaseRunner *test_case_runner;
  IFileWriter *file_writer;
};

} // namespace amelia
