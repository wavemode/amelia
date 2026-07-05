#pragma once

#include "node_type_list.hpp"

namespace amelia {

class Node;

struct AbstractNodeRepository {
  virtual const Node &get_node(NodeId id) const = 0;
};

} // namespace amelia
