#pragma once

#include "prelude.hpp"

#include "util/data/flex.hpp"
#include "util/data/map.hpp"

#include "sema/data/module.hpp"
#include "sema/data/scope.hpp"

namespace amelia {

class Serialize;

struct SemaResult {
  Map<String, ModuleId> module_ids;
  Deque<Module> modules;

  Serialize serialize();
};

} // namespace amelia
