#pragma once

#include "data/core/List.h"

#include "data/core/List.h"
#include "data/core/Slice.h"
#include "data/core/String.h"
#include "data/testing/CompilerTestCase.h"

namespace amelia {

class CompilerTestExecutor {
public:
  void collect_test_cases(String root_directory);
  void execute_test_cases();
  void update_test_cases();

private:
  List<String> paths;
  List<String> file_contents;
  List<CompilerTestCase> test_cases;
};

} // namespace amelia
