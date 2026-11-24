#pragma once

#include "loader/data/loader_result.hpp"
#include "resolver/data/resolved_module_details.hpp"

namespace amelia {

struct ResolverResult {
  LoaderResult loader_result;
  Map<Text, Box<ResolvedModuleDetails>> resolved_modules;
};

} // namespace amelia
