#pragma once

#include <cstddef>

namespace amelia {

struct CompilerTestExecutionOutcome {
  size_t count_executed = 0;
  size_t count_failed = 0;
  size_t count_updated = 0;
};

} // namespace amelia
