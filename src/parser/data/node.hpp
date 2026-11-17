#pragma once

#include "node_type.hpp"
#include "node_type_list.hpp"

namespace amelia {

class Node {
public:
#define X(NODE_TYPE)                                                           \
  Node(TokenId start, TokenId end, NODE_TYPE node)                             \
      : m_start(start), m_end(end), m_type(NodeType::NODE_TYPE),               \
        m_data(move(node)) {}                                                  \
                                                                               \
  NODE_TYPE &as_##NODE_TYPE() {                                                \
    if (m_type != NodeType::NODE_TYPE) {                                       \
      throw_runtime_error("Node type mismatch");                               \
    }                                                                          \
    return m_data.data_##NODE_TYPE;                                            \
  }                                                                            \
                                                                               \
  const NODE_TYPE &as_##NODE_TYPE() const {                                    \
    if (m_type != NodeType::NODE_TYPE) {                                       \
      throw_runtime_error("Node type mismatch");                               \
    }                                                                          \
    return m_data.data_##NODE_TYPE;                                            \
  }
  NODE_TYPE_LIST
#undef X

  ~Node() { m_data.destroy(m_type); }

  Node(const Node &other)
      : m_start(other.m_start), m_end(other.m_end), m_type(other.m_type),
        m_data(other.m_type, other.m_data) {}

  Node(Node &&other) noexcept
      : m_start(other.m_start), m_end(other.m_end), m_type(other.m_type),
        m_data(other.m_type, move(other.m_data)) {}

  NodeType type() const { return m_type; }

  TokenId start_token() const { return m_start; }

  TokenId end_token() const { return m_end; }

  Node &operator=(const Node &other);

  Node &operator=(Node &&other);

private:
  union NodeData {
#define X(NODE_TYPE)                                                           \
  NODE_TYPE data_##NODE_TYPE;                                                  \
                                                                               \
  explicit NodeData(NODE_TYPE node) : data_##NODE_TYPE(move(node)) {}
    NODE_TYPE_LIST
#undef X

    NodeData(NodeType type, const NodeData &other);
    NodeData(NodeType type, NodeData &&other);
    void assign(NodeType type, const NodeData &other);
    void assign(NodeType type, NodeData &&other);
    void destroy(NodeType type) noexcept;
    ~NodeData();
  };

  TokenId m_start;
  TokenId m_end;
  NodeType m_type;
  NodeData m_data;
};

} // namespace amelia
