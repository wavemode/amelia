#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "NodeInfo.h"
#include "data/core/List.h"

#include "NodeTypeList.h"

namespace amelia {

class NodeManager {
public:
  NodeInfo get_node_info(size_t id) const { return nodes[id]; }

#define X(name)                                                                                    \
private:                                                                                           \
  std::unordered_map<size_t, name> name##_nodes;                                                   \
                                                                                                   \
public:                                                                                            \
  const name &get_##name(size_t id) const { return name##_nodes.at(id); }                          \
  size_t add_##name(Location loc, name node) {                                                     \
    size_t id = add_node_info({loc, NodeType::name});                                              \
    name##_nodes[id] = std::move(node);                                                            \
    return id;                                                                                     \
  }
  NODE_TYPE_LIST
#undef X

private:
  List<NodeInfo> nodes;

  size_t add_node_info(NodeInfo info) {
    nodes.push_back(info);
    return nodes.size() - 1;
  }
};

} // namespace amelia
