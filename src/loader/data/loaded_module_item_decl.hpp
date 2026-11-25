#pragma once

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "loader/data/loaded_declaration_visibility.hpp"
#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

using NodeId = int32_t;

struct LoadedModuleItemDecl : AbstractLoadedModuleDecl {
  LoadedDeclarationVisibility visibility;
  List<String> names;
};

} // namespace amelia
