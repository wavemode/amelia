#include "node.hpp"

#include <new>

namespace amelia {

Node &Node::operator=(Node &&other) {
  if (this != &other) {
    m_start = other.m_start;
    m_end = other.m_end;
    if (m_type == other.m_type) {
      m_data.assign(m_type, move(other.m_data));
    } else {
      m_data.destroy(m_type);
      m_type = other.m_type;
      new (&m_data) NodeData(m_type, move(other.m_data));
    }
  }
  return *this;
}

Node &Node::operator=(const Node &other) {
  if (this != &other) {
    m_start = other.m_start;
    m_end = other.m_end;
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

Node::NodeData::NodeData(NodeType type, const NodeData &other) {
  switch (type) {
#define X(NODE_TYPE)                                                           \
  case NodeType::NODE_TYPE:                                                    \
    new (&data_##NODE_TYPE) NODE_TYPE(other.data_##NODE_TYPE);                 \
    break;
    NODE_TYPE_LIST
#undef X
  }
}

Node::NodeData::NodeData(NodeType type, NodeData &&other) {
  switch (type) {
#define X(NODE_TYPE)                                                           \
  case NodeType::NODE_TYPE:                                                    \
    new (&data_##NODE_TYPE) NODE_TYPE(move(other.data_##NODE_TYPE));           \
    break;
    NODE_TYPE_LIST
#undef X
  }
}

void Node::NodeData::assign(NodeType type, const NodeData &other) {
  switch (type) {
#define X(NODE_TYPE)                                                           \
  case NodeType::NODE_TYPE:                                                    \
    data_##NODE_TYPE = other.data_##NODE_TYPE;                                 \
    break;
    NODE_TYPE_LIST
#undef X
  }
}

void Node::NodeData::assign(NodeType type, NodeData &&other) {
  switch (type) {
#define X(NODE_TYPE)                                                           \
  case NodeType::NODE_TYPE:                                                    \
    data_##NODE_TYPE = move(other.data_##NODE_TYPE);                           \
    break;
    NODE_TYPE_LIST
#undef X
  }
}

void Node::NodeData::destroy(NodeType type) noexcept {
  switch (type) {
#define X(NODE_TYPE)                                                           \
  case NodeType::NODE_TYPE:                                                    \
    data_##NODE_TYPE.~NODE_TYPE();                                             \
    break;
    NODE_TYPE_LIST
#undef X
  }
}

Node::NodeData::~NodeData() {}

} // namespace amelia
