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
  NodeId key;
  NodeId value;
};

struct ExpressionStatementNode {
  NodeId expr;
};

struct ObjectLiteralNode {
  List<NodeId> entries;
};

struct IfThenExpressionNode {
  NodeId condition;
  NodeId then_branch;
};

struct IfThenElseExpressionNode {
  NodeId condition;
  NodeId then_branch;
  NodeId else_branch;
};

struct CatchClauseNode {
  NodeId exc_type;
  NodeId body;
};

struct CatchClauseBindingNode {
  NodeId var;
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

struct GreaterEqualsExpressionNode {
  NodeId left;
  NodeId right;
};

struct LessEqualsExpressionNode {
  NodeId left;
  NodeId right;
};

struct GreaterExpressionNode {
  NodeId left;
  NodeId right;
};

struct LessExpressionNode {
  NodeId left;
  NodeId right;
};

struct LeftShiftExpressionNode {
  NodeId left;
  NodeId right;
};

struct RightShiftExpressionNode {
  NodeId left;
  NodeId right;
};

struct AddExpressionNode {
  NodeId left;
  NodeId right;
};

struct SubtractExpressionNode {
  NodeId left;
  NodeId right;
};

struct MultiplyExpressionNode {
  NodeId left;
  NodeId right;
};

struct DivideExpressionNode {
  NodeId left;
  NodeId right;
};

struct ModuloExpressionNode {
  NodeId left;
  NodeId right;
};

struct RefExpressionNode {
  NodeId expr;
};

struct AwaitExpressionNode {
  NodeId expr;
};

struct NotExpressionNode {
  NodeId expr;
};

struct BitwiseNotExpressionNode {
  NodeId expr;
};

struct DerefExpressionNode {
  NodeId expr;
};

struct PositiveExpressionNode {
  NodeId expr;
};

struct NegativeExpressionNode {
  NodeId expr;
};

struct EllipsisExpressionNode {
  NodeId expr;
};

struct FieldAccessExpressionNode {
  NodeId object;
  NodeId field;
};

struct NumericFieldAccessExpressionNode {
  NodeId object;
  TokenId field;
};

struct IndexingExpressionNode {
  NodeId object;
  NodeId index;
};

struct PositionFunctionArgumentNode {
  NodeId expr;
};

struct NamedFunctionArgumentNode {
  NodeId name;
  NodeId expr;
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
  X(IfThenExpressionNode)                                                                          \
  X(IfThenElseExpressionNode)                                                                      \
  X(CatchClauseNode)                                                                               \
  X(CatchClauseBindingNode)                                                                        \
  X(TryCatchExpressionNode)                                                                        \
  X(CaseClauseNode)                                                                                \
  X(SwitchExpressionNode)                                                                          \
  X(OrExpressionNode)                                                                              \
  X(AndExpressionNode)                                                                             \
  X(BitwiseOrExpressionNode)                                                                       \
  X(BitwiseAndExpressionNode)                                                                      \
  X(BitwiseXorExpressionNode)                                                                      \
  X(EqualsExpressionNode)                                                                          \
  X(NotEqualsExpressionNode)                                                                       \
  X(GreaterEqualsExpressionNode)                                                                   \
  X(LessEqualsExpressionNode)                                                                      \
  X(GreaterExpressionNode)                                                                         \
  X(LessExpressionNode)                                                                            \
  X(LeftShiftExpressionNode)                                                                       \
  X(RightShiftExpressionNode)                                                                      \
  X(AddExpressionNode)                                                                             \
  X(SubtractExpressionNode)                                                                        \
  X(MultiplyExpressionNode)                                                                        \
  X(DivideExpressionNode)                                                                          \
  X(ModuloExpressionNode)                                                                          \
  X(RefExpressionNode)                                                                             \
  X(AwaitExpressionNode)                                                                           \
  X(NotExpressionNode)                                                                             \
  X(BitwiseNotExpressionNode)                                                                      \
  X(DerefExpressionNode)                                                                           \
  X(PositiveExpressionNode)                                                                        \
  X(NegativeExpressionNode)                                                                        \
  X(EllipsisExpressionNode)                                                                        \
  X(FieldAccessExpressionNode)                                                                     \
  X(NumericFieldAccessExpressionNode)                                                              \
  X(IndexingExpressionNode)

} // namespace amelia
