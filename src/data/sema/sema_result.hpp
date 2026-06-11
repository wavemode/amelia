#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"
#include "data/util/map.hpp"

#include "data/sema/module.hpp"
#include "data/sema/scope.hpp"

namespace amelia {

struct SemaResult {
  Map<String, ModuleId> module_ids;
  Deque<Module> modules;
};

void format_sema_result(AbstractString &out, const SemaResult &sema_result);

} // namespace amelia
