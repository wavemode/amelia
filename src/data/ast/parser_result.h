#pragma once

#include <cstddef>
#include <vector>

#include "data/util/list.h"
#include "data/util/map.h"
#include "node_info.h"
#include "node_type_list.h"

namespace amelia {

class ParserResult {
public:
  NodeInfo get_node_info(size_t id) const { return m_nodes[id]; }

#define X(NODE_TYPE)                                                                               \
private:                                                                                           \
  Map<size_t, NODE_TYPE> m_##NODE_TYPE##_nodes;                                                    \
                                                                                                   \
public:                                                                                            \
  const NODE_TYPE &get_##NODE_TYPE(size_t id) const { return m_##NODE_TYPE##_nodes.get(id); }      \
                                                                                                   \
private:                                                                                           \
  size_t add_##NODE_TYPE(Location loc, NODE_TYPE node) {                                           \
    size_t id = add_node_info({loc, NodeType::NODE_TYPE});                                         \
    m_##NODE_TYPE##_nodes.set(id, std::move(node));                                                \
    return id;                                                                                     \
  }
  NODE_TYPE_LIST
#undef X

private:
  List<NodeInfo> m_nodes;

  size_t add_node_info(NodeInfo info) {
    m_nodes.push_back(info);
    return m_nodes.size() - 1;
  }
};

} // namespace amelia
