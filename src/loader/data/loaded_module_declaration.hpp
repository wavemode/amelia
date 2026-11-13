#pragma once

#include "loaded_module_declaration_visibility.hpp"
#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

using NodeId = int32_t;

struct LoadedModuleDeclaration {
  String name;
  LoadedModuleDeclarationVisibility visibility;
  List<NodeId> nodes;
};

} // namespace amelia
