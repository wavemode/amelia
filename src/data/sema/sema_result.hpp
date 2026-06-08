#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"
#include "data/util/map.hpp"

#include "data/sema/module.hpp"
#include "data/sema/module_metadata.hpp"

namespace amelia {

struct SemaResult {
  Map<String, ModuleId> module_ids;
  List<FlexShared<Module>> modules;
  List<ModuleMetadata> module_meta;
};

} // namespace amelia
