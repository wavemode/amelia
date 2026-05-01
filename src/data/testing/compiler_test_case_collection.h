#pragma once

#include "data/core/list.h"
#include "data/core/string.h"
#include "data/testing/compiler_test_case.h"

namespace amelia {

struct CompilerTestCaseCollection {
  List<CompilerTestCase> test_cases;
  List<String> file_contents;
  List<String> paths;
};

} // namespace amelia
