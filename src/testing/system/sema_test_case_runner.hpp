#pragma once

#include "fs/interface/file_loader.hpp"
#include "testing/interface/test_case_runner.hpp"

namespace amelia {

class SemaTestCaseRunner : public ITestCaseRunner {
public:
  SemaTestCaseRunner(IFileLoader &file_loader);

  void run_test_case(AbstractString &output, CompilerTestCase test_case) override;

private:
  IFileLoader &m_file_loader;
};

} // namespace amelia
