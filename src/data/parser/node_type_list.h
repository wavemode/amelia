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

struct ConstStatementNode {
  NodeId target;
  NodeId expression;
};

struct StringLiteralNode {
  TokenId value;
};

struct NumberLiteralNode {
  TokenId value;
};

#define NODE_TYPE_LIST                                                                             \
  X(ModuleNode)                                                                                    \
  X(IdentifierNode)                                                                                \
  X(LetStatementNode)                                                                              \
  X(ConstStatementNode)                                                                            \
  X(StringLiteralNode)                                                                             \
  X(NumberLiteralNode)

} // namespace amelia
