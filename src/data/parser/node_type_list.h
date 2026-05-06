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

struct ParenthesizedExpressionNode {
  List<NodeId> exprs;
};

struct ArrayLiteralNode {
  List<NodeId> exprs;
};

struct BlockExpressionNode {
  List<NodeId> stmts;
};

struct KeyValueEntryNode {
  TokenId field;
  NodeId value;
};

struct ObjectLiteralNode {
  List<KeyValueEntryNode> entries;
};

#define NODE_TYPE_LIST                                                                             \
  X(ModuleNode)                                                                                    \
  X(IdentifierNode)                                                                                \
  X(LetStatementNode)                                                                              \
  X(ConstStatementNode)                                                                            \
  X(StringLiteralNode)                                                                             \
  X(NumberLiteralNode)                                                                             \
  X(ParenthesizedExpressionNode)                                                                   \
  X(ArrayLiteralNode)                                                                              \
  X(BlockExpressionNode)                                                                           \
  X(ObjectLiteralNode)

} // namespace amelia
