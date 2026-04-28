#pragma once

#include "NodeTypeList.h"

namespace amelia {

enum class NodeType {
#define X(name) name,
  NODE_TYPE_LIST
#undef X
};

} // namespace amelia
