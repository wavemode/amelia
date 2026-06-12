#pragma once

#include "data/sema/sema_result.hpp"

namespace amelia {

struct Analyzer {

  static void typecheck(SemaResult &sema_result);
};

} // namespace amelia
