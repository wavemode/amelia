#pragma once

#include "node_type_list.hpp"
#include "prelude.hpp"

namespace amelia {

enum class NodeType {
#define X(NODE_TYPE) NODE_TYPE,
  NODE_TYPE_LIST
#undef X
};

inline void node_type_to_string(AbstractString &out, NodeType type) {
  switch (type) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE:                                                                        \
    out.append(#NODE_TYPE);                                                                        \
    break;
    NODE_TYPE_LIST
#undef X
  }
}

} // namespace amelia
