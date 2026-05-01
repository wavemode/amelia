#pragma once

#include "data/testing/compiler_test_case.h"
#include "data/util/list.h"
#include "data/util/string.h"

namespace amelia {

struct CompilerTestCaseCollection {
  List<CompilerTestCase> test_cases;
  List<String> file_contents;
  List<String> paths;
};

} // namespace amelia
