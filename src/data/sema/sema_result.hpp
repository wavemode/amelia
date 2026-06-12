#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"
#include "data/util/map.hpp"

#include "data/sema/module.hpp"
#include "data/sema/scope.hpp"
#include "data/testing/pretty_print.hpp"

namespace amelia {

struct SemaResult {
  Map<String, ModuleId> module_ids;
  Deque<Module> modules;

  PrettyPrint pretty_print();
};

} // namespace amelia
