#pragma once

#include "resolved_module_details.hpp"
#include "util/data/option.hpp"
#include "util/data/ref.hpp"
#include "util/data/text.hpp"

namespace amelia {

struct ResolvedModuleImport : AbstractResolvedModuleItem {
  Option<Ref<ResolvedModuleDetails>> module;
  Text name;
  Option<Text> alias;
};

} // namespace amelia
