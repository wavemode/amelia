#pragma once

namespace amelia {

class Node;

struct AbstractNodeRepository {
  virtual const Node &get_node(NodeId id) const = 0;
};

} // namespace amelia
