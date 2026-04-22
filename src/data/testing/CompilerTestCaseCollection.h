#pragma once

#include "data/core/List.h"

#include "data/core/List.h"
#include "data/core/String.h"
#include "data/testing/CompilerTestCase.h"

namespace amelia {

struct CompilerTestCaseCollection {
  List<CompilerTestCase> test_cases;
  List<String> file_contents;
  List<String> paths;
};

} // namespace amelia
