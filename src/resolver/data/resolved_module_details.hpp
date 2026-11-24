#pragma once

#include "abstract_resolved_module_item.hpp"
#include "loader/data/loaded_module_details.hpp"
#include "util/data/map.hpp"
#include "util/data/option.hpp"
#include "util/data/ref.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct ResolvedModuleDetails {
  Option<Ref<LoadedModuleDetails>> details;
  Map<Text, Box<AbstractResolvedModuleItem>> items;
};

} // namespace amelia
