#pragma once

#include <cstddef>

namespace amelia {

class Node;
using NodeId = size_t;

struct AbstractNodeRepository {
  virtual const Node &get_node(NodeId id) const = 0;
};

} // namespace amelia
