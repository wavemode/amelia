#pragma once

#include "data/util/abstract_string.h"
#include "node_type_list.h"

namespace amelia {

enum class NodeType {
#define X(name) name,
  NODE_TYPE_LIST
#undef X
};

} // namespace amelia
