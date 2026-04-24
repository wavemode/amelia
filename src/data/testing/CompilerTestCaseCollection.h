#pragma once

#include "data/core/List.h"
#include "data/testing/CompilerTestCase.h"
#include "data/text/String.h"

namespace amelia {

struct CompilerTestCaseCollection {
  List<CompilerTestCase> test_cases;
  List<String> file_contents;
  List<String> paths;
};

} // namespace amelia
