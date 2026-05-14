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

struct PreIncrementStatementNode {
  NodeId target;
};

struct PostIncrementStatementNode {
  NodeId target;
};

struct PreDecrementStatementNode {
  NodeId target;
};

struct PostDecrementStatementNode {
  NodeId target;
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

struct AssignmentStatementNode {
  NodeId target;
  NodeId expression;
};

struct AddAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct SubAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct MulAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct DivAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct ModAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct LeftShiftAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct RightShiftAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct BitwiseAndAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct BitwiseOrAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct BitwiseXorAssignStatementNode {
  NodeId target;
  NodeId expression;
};

struct ObjectLiteralNode {
  List<NodeId> entries;
};

struct IfThenStatementNode {
  NodeId condition;
  NodeId then_branch;
};

struct IfThenElseStatementNode {
  NodeId condition;
  NodeId then_branch;
  NodeId else_branch;
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

struct PositionalFunctionArgumentNode {
  NodeId expr;
};

struct NamedFunctionArgumentNode {
  NodeId name;
  NodeId expr;
};

struct FunctionCallExpressionNode {
  NodeId callee;
  List<NodeId> args;
};

struct ScopeResolutionExpressionNode {
  NodeId scope;
  NodeId name;
};

struct BlockStatementNode {
  List<NodeId> stmts;
};

struct ThrowStatementNode {
  NodeId expr;
};

struct OperatorIdentAddNode {};

struct OperatorIdentSubNode {};

struct OperatorIdentStarNode {};

struct OperatorIdentDivNode {};

struct OperatorIdentModNode {};

struct OperatorIdentIncNode {};

struct OperatorIdentDecNode {};

struct OperatorIdentEqNode {};

struct OperatorIdentNeqNode {};

struct OperatorIdentGtNode {};

struct OperatorIdentLtNode {};

struct OperatorIdentGteNode {};

struct OperatorIdentLteNode {};

struct OperatorIdentNotNode {};

struct OperatorIdentAndNode {};

struct OperatorIdentOrNode {};

struct OperatorIdentBitwiseNotNode {};

struct OperatorIdentBitwiseAndNode {};

struct OperatorIdentBitwiseOrNode {};

struct OperatorIdentBitwiseXorNode {};

struct OperatorIdentLeftShiftNode {};

struct OperatorIdentRightShiftNode {};

struct OperatorIdentAssignNode {};

struct OperatorIdentAddAssignNode {};

struct OperatorIdentSubAssignNode {};

struct OperatorIdentMulAssignNode {};

struct OperatorIdentDivAssignNode {};

struct OperatorIdentModAssignNode {};

struct OperatorIdentBitwiseAndAssignNode {};

struct OperatorIdentBitwiseOrAssignNode {};

struct OperatorIdentBitwiseXorAssignNode {};

struct OperatorIdentLeftShiftAssignNode {};

struct OperatorIdentRightShiftAssignNode {};

struct OperatorIdentIxNode {};

struct OperatorIdentFuncallNode {};

struct OperatorIdentAsNode {
  NodeId type;
};

struct OperatorIdentifierNode {
  NodeId operator_node;
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
  X(IfThenStatementNode)                                                                           \
  X(IfThenElseStatementNode)                                                                       \
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
  X(IndexingExpressionNode)                                                                        \
  X(PositionalFunctionArgumentNode)                                                                \
  X(NamedFunctionArgumentNode)                                                                     \
  X(FunctionCallExpressionNode)                                                                    \
  X(ScopeResolutionExpressionNode)                                                                 \
  X(PreIncrementStatementNode)                                                                     \
  X(PostIncrementStatementNode)                                                                    \
  X(PreDecrementStatementNode)                                                                     \
  X(PostDecrementStatementNode)                                                                    \
  X(BlockStatementNode)                                                                            \
  X(ThrowStatementNode)                                                                            \
  X(OperatorIdentAddNode)                                                                          \
  X(OperatorIdentSubNode)                                                                          \
  X(OperatorIdentStarNode)                                                                         \
  X(OperatorIdentDivNode)                                                                          \
  X(OperatorIdentModNode)                                                                          \
  X(OperatorIdentIncNode)                                                                          \
  X(OperatorIdentDecNode)                                                                          \
  X(OperatorIdentEqNode)                                                                           \
  X(OperatorIdentNeqNode)                                                                          \
  X(OperatorIdentGtNode)                                                                           \
  X(OperatorIdentLtNode)                                                                           \
  X(OperatorIdentGteNode)                                                                          \
  X(OperatorIdentLteNode)                                                                          \
  X(OperatorIdentNotNode)                                                                          \
  X(OperatorIdentAndNode)                                                                          \
  X(OperatorIdentOrNode)                                                                           \
  X(OperatorIdentBitwiseNotNode)                                                                   \
  X(OperatorIdentBitwiseAndNode)                                                                   \
  X(OperatorIdentBitwiseOrNode)                                                                    \
  X(OperatorIdentBitwiseXorNode)                                                                   \
  X(OperatorIdentLeftShiftNode)                                                                    \
  X(OperatorIdentRightShiftNode)                                                                   \
  X(OperatorIdentAssignNode)                                                                       \
  X(OperatorIdentAddAssignNode)                                                                    \
  X(OperatorIdentSubAssignNode)                                                                    \
  X(OperatorIdentMulAssignNode)                                                                    \
  X(OperatorIdentDivAssignNode)                                                                    \
  X(OperatorIdentModAssignNode)                                                                    \
  X(OperatorIdentBitwiseAndAssignNode)                                                             \
  X(OperatorIdentBitwiseOrAssignNode)                                                              \
  X(OperatorIdentBitwiseXorAssignNode)                                                             \
  X(OperatorIdentLeftShiftAssignNode)                                                              \
  X(OperatorIdentRightShiftAssignNode)                                                             \
  X(OperatorIdentIxNode)                                                                           \
  X(OperatorIdentFuncallNode)                                                                      \
  X(OperatorIdentAsNode)                                                                           \
  X(OperatorIdentifierNode)                                                                        \
  X(AssignmentStatementNode)                                                                       \
  X(AddAssignStatementNode)                                                                        \
  X(SubAssignStatementNode)                                                                        \
  X(MulAssignStatementNode)                                                                        \
  X(DivAssignStatementNode)                                                                        \
  X(ModAssignStatementNode)                                                                        \
  X(LeftShiftAssignStatementNode)                                                                  \
  X(RightShiftAssignStatementNode)                                                                 \
  X(BitwiseAndAssignStatementNode)                                                                 \
  X(BitwiseOrAssignStatementNode)                                                                  \
  X(BitwiseXorAssignStatementNode)

} // namespace amelia
