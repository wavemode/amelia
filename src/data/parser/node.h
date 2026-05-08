#pragma once

#include "data/source/location.h"
#include "node_type.h"
#include "node_type_list.h"
#include "prelude.h"

namespace amelia {

class Node {
public:
#define X(NODE_TYPE)                                                                               \
  Node(Location loc, NODE_TYPE node)                                                               \
      : m_type(NodeType::NODE_TYPE), m_loc(loc), m_data(std::move(node)) {}                        \
                                                                                                   \
  NODE_TYPE &as_##NODE_TYPE() {                                                                    \
    if (m_type != NodeType::NODE_TYPE) {                                                           \
      throw RuntimeError("Node type mismatch");                                                    \
    }                                                                                              \
    return m_data.data_##NODE_TYPE;                                                                \
  }                                                                                                \
                                                                                                   \
  const NODE_TYPE &as_##NODE_TYPE() const {                                                        \
    if (m_type != NodeType::NODE_TYPE) {                                                           \
      throw RuntimeError("Node type mismatch");                                                    \
    }                                                                                              \
    return m_data.data_##NODE_TYPE;                                                                \
  }
  NODE_TYPE_LIST
#undef X

  ~Node() {
    m_data.destroy(m_type);
  }

  Node(const Node &other)
      : m_loc(other.m_loc), m_type(other.m_type), m_data(other.m_type, other.m_data) {}

  Node(Node &&other) noexcept
      : m_loc(std::move(other.m_loc)), m_type(other.m_type),
        m_data(other.m_type, std::move(other.m_data)) {}

  NodeType type() const {
    return m_type;
  }

  Location location() const {
    return m_loc;
  }

  Node &operator=(const Node &other) {
    if (this != &other) {
      m_loc = other.m_loc;
      if (m_type == other.m_type) {
        m_data.assign(m_type, other.m_data);
      } else {
        m_data.destroy(m_type);
        m_type = other.m_type;
        new (&m_data) NodeData(m_type, other.m_data);
      }
    }
    return *this;
  }

  Node &operator=(Node &&other) {
    if (this != &other) {
      m_loc = std::move(other.m_loc);
      if (m_type == other.m_type) {
        m_data.assign(m_type, std::move(other.m_data));
      } else {
        m_data.destroy(m_type);
        m_type = other.m_type;
        new (&m_data) NodeData(m_type, std::move(other.m_data));
      }
    }
    return *this;
  }

private:
  union NodeData {
#define X(NODE_TYPE)                                                                               \
  NODE_TYPE data_##NODE_TYPE;                                                                      \
                                                                                                   \
  explicit NodeData(NODE_TYPE node) : data_##NODE_TYPE(std::move(node)) {}
    NODE_TYPE_LIST
#undef X

    NodeData(NodeType type, const NodeData &other) {
      switch (type) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE:                                                                        \
    new (&data_##NODE_TYPE) NODE_TYPE(other.data_##NODE_TYPE);                                     \
    break;
        NODE_TYPE_LIST
#undef X
      }
    }

    NodeData(NodeType type, NodeData &&other) {
      switch (type) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE:                                                                        \
    new (&data_##NODE_TYPE) NODE_TYPE(std::move(other.data_##NODE_TYPE));                          \
    break;
        NODE_TYPE_LIST
#undef X
      }
    }

    void assign(NodeType type, const NodeData &other) {
      switch (type) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE:                                                                        \
    data_##NODE_TYPE = other.data_##NODE_TYPE;                                                     \
    break;
        NODE_TYPE_LIST
#undef X
      }
    }

    void assign(NodeType type, NodeData &&other) {
      switch (type) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE:                                                                        \
    data_##NODE_TYPE = std::move(other.data_##NODE_TYPE);                                          \
    break;
        NODE_TYPE_LIST
#undef X
      }
    }

    void destroy(NodeType type) noexcept {
      switch (type) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE:                                                                        \
    data_##NODE_TYPE.~NODE_TYPE();                                                                 \
    break;
        NODE_TYPE_LIST
#undef X
      }
    }

    ~NodeData() {}
  };

  Location m_loc;
  NodeType m_type;
  NodeData m_data;
};

} // namespace amelia
