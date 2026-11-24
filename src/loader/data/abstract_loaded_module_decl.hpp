#pragma once

#include "loader/data/loaded_declaration_visibility.hpp"
#include "util/data/list.hpp"
#include "util/data/utility.hpp"

namespace amelia {

using NodeId = int32_t;

struct AbstractLoadedModuleDecl : Dynamic {
  NodeId node_id = -1;
  List<NodeId> annotations;
  LoadedDeclarationVisibility visibility;
};

} // namespace amelia
