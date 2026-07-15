#pragma once

#include <cstdint>

namespace amelia {

class Node;

using NodeId = int32_t;

struct AbstractNodeRepository {
  virtual const Node &get_node(NodeId id) const = 0;
};

} // namespace amelia
