#pragma once

#include "loader/data/loaded_module_decl_visibility.hpp"
#include "util/data/string.hpp"
#include "util/data/utility.hpp"

namespace amelia {

struct AbstractLoadedModuleDecl : Dynamic {
  String name;
  LoadedModuleDeclarationVisibility visibility;
};

} // namespace amelia
