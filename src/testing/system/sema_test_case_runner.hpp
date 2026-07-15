#pragma once

#include "testing/interface/test_case_runner.hpp"
#include "util/interface/file_loader.hpp"

namespace amelia {

class SemaTestCaseRunner : public ITestCaseRunner {
public:
  SemaTestCaseRunner(IFileLoader &file_loader);

  void run_test_case(AbstractString &output, CompilerTestCase test_case) override;

private:
  IFileLoader &m_file_loader;
};

} // namespace amelia
