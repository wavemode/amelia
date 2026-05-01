#pragma once

#include "data/core/list.h"
#include "data/testing/compiler_test_case.h"
#include "data/text/string.h"

namespace amelia {

struct CompilerTestCaseCollection {
  List<CompilerTestCase> test_cases;
  List<String> file_contents;
  List<String> paths;
};

} // namespace amelia
