#include "CompilerTestExecutor.h"

#include <filesystem>
#include <iostream>

namespace amelia {

void CompilerTestExecutor::collect_test_cases(String root) {
  std::error_code ec;
  for (std::filesystem::recursive_directory_iterator it(root.c_str(), ec), end; it != end;
       it.increment(ec)) {
    if (ec) {
      ec.clear();
      continue;
    }
    if (!it->is_regular_file())
      continue;
  }
}

void CompilerTestExecutor::execute_test_cases() {}

void CompilerTestExecutor::update_test_cases() {}

} // namespace amelia
