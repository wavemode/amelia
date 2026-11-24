#pragma once

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "loader/data/loaded_module_details.hpp"

namespace amelia {

struct LoadedModuleSubmoduleDecl : AbstractLoadedModuleDecl {
  LoadedModuleDetails details;
  Option<Ref<LoadedModuleDetails>> parent;
};

} // namespace amelia
