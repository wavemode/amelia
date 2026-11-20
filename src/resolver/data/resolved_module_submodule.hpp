#pragma once

#include "resolved_module_details.hpp"
#include "util/data/option.hpp"
#include "util/data/ref.hpp"

namespace amelia {

struct ResolvedModuleSubmodule : AbstractResolvedModuleItem {
  Option<Ref<ResolvedModuleDetails>> details;
};

} // namespace amelia
