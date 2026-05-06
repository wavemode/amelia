#pragma once

#include <cstddef>

#include "data/util/list.h"

namespace amelia {

using TokenId = size_t;
using NodeId = size_t;

struct ModuleNode {
  List<NodeId> decls;
};

struct IdentifierNode {
  TokenId name;
};

struct LetStatementNode {
  NodeId target;
  NodeId expression;
};

#define NODE_TYPE_LIST                                                                             \
  X(ModuleNode)                                                                                    \
  X(IdentifierNode)                                                                                \
  X(LetStatementNode)

} // namespace amelia
