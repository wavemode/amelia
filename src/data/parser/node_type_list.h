#pragma once

#include <cstddef>

#include "data/util/list.h"
#include "data/util/option.h"

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
  TokenId key;
  NodeId value;
};

struct ExpressionStatementNode {
  NodeId expr;
};

struct ObjectLiteralNode {
  List<NodeId> entries;
};

struct IfExpressionNode {
  NodeId condition;
  NodeId then_branch;
  Option<NodeId> else_branch;
};

struct CatchClauseNode {
  Option<TokenId> var;
  NodeId exc_type;
  NodeId body;
};

struct TryCatchExpressionNode {
  NodeId try_block;
  List<NodeId> clauses;
};

struct CaseClauseNode {
  NodeId expr;
  NodeId body;
};

struct SwitchExpressionNode {
  NodeId expr;
  List<NodeId> clauses;
};

struct OrExpressionNode {
  NodeId left;
  NodeId right;
};

struct AndExpressionNode {
  NodeId left;
  NodeId right;
};

struct BitwiseOrExpressionNode {
  NodeId left;
  NodeId right;
};

struct BitwiseAndExpressionNode {
  NodeId left;
  NodeId right;
};

struct BitwiseXorExpressionNode {
  NodeId left;
  NodeId right;
};

struct EqualsExpressionNode {
  NodeId left;
  NodeId right;
};

struct NotEqualsExpressionNode {
  NodeId left;
  NodeId right;
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
  X(KeyValueEntryNode)                                                                             \
  X(ObjectLiteralNode)                                                                             \
  X(ExpressionStatementNode)                                                                       \
  X(IfExpressionNode)                                                                              \
  X(CatchClauseNode)                                                                               \
  X(TryCatchExpressionNode)                                                                        \
  X(CaseClauseNode)                                                                                \
  X(SwitchExpressionNode)                                                                          \
  X(OrExpressionNode)                                                                              \
  X(AndExpressionNode)                                                                             \
  X(BitwiseOrExpressionNode)                                                                       \
  X(BitwiseAndExpressionNode)                                                                      \
  X(BitwiseXorExpressionNode)                                                                      \
  X(EqualsExpressionNode)                                                                          \
  X(NotEqualsExpressionNode)

} // namespace amelia
