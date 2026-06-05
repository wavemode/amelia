#pragma once

#include "prelude.hpp"

#include "data/parser/abstract_node_repository.hpp"
#include "data/parser/node.hpp"

namespace amelia {

class ParserResult : public AbstractNodeRepository {
public:
  const Node &get_node(NodeId id) const override {
    if (static_cast<size_t>(id) >= m_nodes.size()) {
      throw RuntimeError("Invalid node ID");
    }
    return m_nodes[static_cast<size_t>(id)];
  }

  template <typename NT> NodeId add_node(TokenId start, TokenId end, NT node) {
    NodeId id = m_nodes.size();
    m_nodes.emplace_back(start, end, move(node));
    return id;
  }

  Slice<Node> nodes() noexcept {
    return m_nodes.data();
  }

  ConstSlice<Node> nodes() const noexcept {
    return m_nodes.data();
  }

private:
  List<Node> m_nodes;
};

} // namespace amelia
