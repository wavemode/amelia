#pragma once

#include "data/source/location.h"
#include "node_type.h"

namespace amelia {

struct NodeInfo {
  Location location;
  NodeType type;
};

} // namespace amelia
