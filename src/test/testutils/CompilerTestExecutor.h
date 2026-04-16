#pragma once

#include <vector>

#include "CompilerTestCase.h"
#include "util/slice/Slice.h"
#include "util/text/String.h"

namespace amelia {

class CompilerTestExecutor {
public:
  void collect_test_cases(String root_directory);
  void execute_test_cases();
  void update_test_cases();

private:
  std::vector<String> paths;
  std::vector<String> file_contents;
  std::vector<CompilerTestCase> test_cases;
};

} // namespace amelia
