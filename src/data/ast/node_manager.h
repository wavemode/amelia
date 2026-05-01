#pragma once

#include <cstddef>
#include <vector>

#include "data/core/list.h"
#include "data/core/map.h"
#include "node_info.h"
#include "node_type_list.h"

namespace amelia {

class NodeManager {
public:
  NodeInfo get_node_info(size_t id) const { return m_nodes[id]; }

#define X(name)                                                                                    \
private:                                                                                           \
  Map<size_t, name> m_##name##_nodes;                                                              \
                                                                                                   \
public:                                                                                            \
  const name &get_##name(size_t id) const { return m_##name##_nodes.get(id); }                     \
  size_t add_##name(Location loc, name node) {                                                     \
    size_t id = add_node_info({loc, NodeType::name});                                              \
    m_##name##_nodes.set(id, std::move(node));                                                     \
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
