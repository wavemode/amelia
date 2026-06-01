#pragma once

#include <cstddef>
#include <vector>

#include "data/parser/abstract_node_repository.hpp"
#include "data/parser/node.hpp"
#include "prelude.hpp"

namespace amelia {

class ParserResult : public AbstractNodeRepository {
public:
  const Node &get_node(NodeId id) const override {
    if (id >= static_cast<NodeId>(m_nodes.size())) {
      throw std::runtime_error("Invalid node ID");
    }
    return m_nodes[id];
  }

  template <typename NT> NodeId add_node(Location loc, NT node) {
    NodeId id = m_nodes.size();
    m_nodes.emplace_back(loc, std::move(node));
    return id;
  }

private:
  List<Node> m_nodes;
};

} // namespace amelia
