#pragma once

#include "data/testing/compiler_test_case.hpp"
#include "data/util/list.hpp"
#include "data/util/string.hpp"

namespace amelia {

struct CompilerTestCaseCollection {
  List<CompilerTestCase> test_cases;
  List<String> file_contents;
  List<String> paths;
};

} // namespace amelia
