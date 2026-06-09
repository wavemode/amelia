#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"
#include "data/util/map.hpp"

#include "data/sema/module_metadata.hpp"
#include "data/sema/scope.hpp"

namespace amelia {

struct SemaResult {
  Map<String, ModuleId> module_ids;
  List<FlexShared<Scope>> module_scopes;
  Deque<ModuleMetadata> module_meta;
};

} // namespace amelia
