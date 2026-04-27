#pragma once

#include <cstddef>

#include "data/testing/CompilerTestCaseCollection.h"

#include "interface/fs/IFileWriter.h"
#include "interface/sys/IEnvironmentReader.h"
#include "interface/sys/IPrinter.h"
#include "interface/testing/ITestCaseRunner.h"

namespace amelia {

struct CompilerTestExecutionOutcome;

class CompilerTestCaseExecutor {
public:
  CompilerTestCaseExecutor(
      ITestCaseRunner *test_case_runner,
      IFileWriter *file_writer,
      IPrinter *printer,
      IEnvironmentReader *env_reader
  );

  CompilerTestExecutionOutcome execute_collection(const CompilerTestCaseCollection &collection);

  bool execute_test_case(const CompilerTestCase &test_case);
  bool update_expected_output(const CompilerTestCase &test_case);

private:
  ITestCaseRunner *test_case_runner;
  IFileWriter *file_writer;
  IPrinter *printer;
  IEnvironmentReader *env_reader;
};

} // namespace amelia
