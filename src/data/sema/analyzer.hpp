#pragma once

#include "data/sema/sema_result.hpp"

namespace amelia {

struct Analyzer {
  static void analyze(SemaResult &sema_result);
};

} // namespace amelia
