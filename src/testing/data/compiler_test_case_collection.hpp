#pragma once

#include "testing/data/compiler_test_case.hpp"
#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct CompilerTestCaseCollection {
  List<CompilerTestCase> test_cases;
  List<String> file_contents;
  List<String> paths;
};

} // namespace amelia
