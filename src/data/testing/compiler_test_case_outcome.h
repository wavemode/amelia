#pragma once

#include <cstddef>

namespace amelia {

struct CompilerTestExecutionOutcome {
  size_t count_executed;
  size_t count_failed;
  size_t count_updated;
};

} // namespace amelia
