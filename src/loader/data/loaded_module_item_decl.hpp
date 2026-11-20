#pragma once

#include "loader/data/loaded_declaration_visibility.hpp"
#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

using NodeId = int32_t;

struct LoadedModuleItemDecl {
  NodeId node_id;
  LoadedDeclarationVisibility visibility;
  List<String> names;
  List<NodeId> annotations;
};

} // namespace amelia
