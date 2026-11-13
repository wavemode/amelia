#pragma once

#include <cstdint>

#include "loader/data/abstract_loaded_module_decl.hpp"

namespace amelia {

using NodeId = int32_t;

struct LoadedModuleItemDecl : AbstractLoadedModuleDecl {
  NodeId node_id;
};

} // namespace amelia
