#pragma once

#include <cstdint>

#include "data/lexer/token_type.h"
#include "data/util/list.h"
#include "data/util/option.h"

namespace amelia {

using NodeId = int32_t;

struct ModuleNode {
  List<NodeId> decls;
};

struct EmptyStatementNode {};

struct IdentifierNode {
  TokenId token;
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

struct OperatorIdentAmpersandNode {};

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

struct OperatorIdentCopyAssignNode {};

struct OperatorIdentMoveAssignNode {};

struct OperatorIdentAsNode {
  NodeId type;
};

struct OperatorIdentifierNode {
  NodeId op;
};

struct OperatorFunctionDeclarationNode {
  NodeId operator_ident;
  NodeId signature;
  Option<NodeId> body;
};

struct LetDeclarationNode {
  NodeId target;
  Option<NodeId> type;
  Option<NodeId> expression;
};

struct ConstDeclarationNode {
  NodeId target;
  Option<NodeId> type;
  Option<NodeId> expression;
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
  TokenId lit;
};

struct NumberLiteralNode {
  TokenId lit;
};

struct ParenthesizedExpressionNode {
  List<NodeId> exprs;
};

struct BracketExpressionNode {
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

struct IfStatementNode {
  NodeId introductory_bindings;
  NodeId condition;
  NodeId then_branch;
  Option<NodeId> else_branch;
};

struct IfExpressionNode {
  NodeId introductory_bindings;
  NodeId condition;
  NodeId then_branch;
  NodeId else_branch;
};

struct CatchClauseNode {
  NodeId exc_type;
  Option<NodeId> var;
  NodeId body;
};

struct TryExpressionNode {
  NodeId try_block;
  List<NodeId> clauses;
};

struct TryStatementNode {
  NodeId try_block;
  List<NodeId> clauses;
};

struct CaseClauseNode {
  NodeId introductory_bindings;
  List<NodeId> exprs;
  NodeId body;
};

struct SwitchExpressionNode {
  NodeId introductory_bindings;
  NodeId expr;
  List<NodeId> clauses;
  Option<NodeId> default_body;
};

struct SwitchStatementNode {
  NodeId introductory_bindings;
  NodeId expr;
  List<NodeId> clauses;
  Option<NodeId> default_body;
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
  bool is_const;
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
  bool is_const;
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
  TokenId lit;
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
  Option<NodeId> expr;
};

struct ForInStatementNode {
  NodeId introductory_bindings;
  List<NodeId> vars;
  NodeId iterable;
  NodeId body;
};

struct WhileStatementNode {
  NodeId introductory_bindings;
  NodeId condition;
  NodeId body;
};

struct ContinueStatementNode {};

struct LabelStatementNode {
  NodeId label;
};

struct GotoStatementNode {
  NodeId label;
};

struct ReturnStatementNode {
  Option<NodeId> expr;
};

struct FunctionParameterNode {
  bool variadic;
  NodeId name;
  Option<NodeId> type;
  Option<NodeId> default_value;
};

struct ImplicitParameterListNode {
  List<NodeId> parameters;
};

enum class FunctionCaptureKind {
  Move,
  Copy,
  Ref,
};

struct FunctionSignatureCaptureAnnotationNode {
  FunctionCaptureKind kind;
  NodeId var;
};

struct FunctionSignatureCaptureAnnotationListNode {
  List<NodeId> captures;
};

struct FunctionSignatureNode {
  Option<NodeId> generic_parameter_list;
  List<NodeId> parameters;
  Option<NodeId> implicit_parameter_list;
  Option<NodeId> capture_list;
  Option<NodeId> return_type;
};

struct FunctionBodyNode {
  Option<NodeId> expression;
  Option<List<NodeId>> stmts;
  bool is_default;
  bool is_deleted;
};

struct FunctionDeclarationNode {
  NodeId name;
  NodeId signature;
  Option<NodeId> body;
};

struct FunctionExpressionNode {
  NodeId signature;
  NodeId body;
};

struct LambdaExpressionNode {
  List<NodeId> parameters;
  NodeId body;
};

struct TypeDeclarationNode {
  NodeId name;
  NodeId type_expr;
};

struct IntroductoryBindingsNode {
  List<NodeId> bindings;
};

struct ClassStaticDeclarationNode {
  NodeId decl;
};

struct ClassConstDeclarationNode {
  NodeId decl;
};

struct ClassFieldNode {
  NodeId name;
  Option<NodeId> type;
  Option<NodeId> initializer;
};

struct ClassBaseClassListNode {
  List<NodeId> base_classes;
};

struct ClassDeclarationNode {
  NodeId name;
  Option<NodeId> generic_parameter_list;
  Option<NodeId> base_class_list;
  Option<NodeId> implicit_parameter_list;
  List<NodeId> decls;
};

struct GenericParameterNode {
  bool is_const;
  NodeId name;
  Option<NodeId> constraint;
};

struct TypeConstraintNode {
  NodeId type;
  Option<NodeId> constraint;
};

struct GenericParameterListNode {
  List<NodeId> parameters;
  Option<List<NodeId>> additional_constraints;
};

struct BooleanLiteralNode {
  bool value;
};

struct BoolTypeNode {};

struct ThisLiteralNode {};

struct CopyCtorNameNode {};

struct MoveCtorNameNode {};

struct ClassConstructorNode {
  NodeId name;
  NodeId signature;
  Option<NodeId> body;
};

enum class DeclarationVisibility {
  Public,
  Private,
  Protected,
  Local,
};

struct VisibilityNode {
  DeclarationVisibility visibility;
  NodeId decl;
};

struct CopyExpressionNode {
  NodeId expr;
};

struct MoveExpressionNode {
  NodeId expr;
};

#define NODE_TYPE_LIST                                                                             \
  X(ModuleNode)                                                                                    \
  X(IdentifierNode)                                                                                \
  X(EmptyStatementNode)                                                                            \
  X(LetDeclarationNode)                                                                            \
  X(ConstDeclarationNode)                                                                          \
  X(StringLiteralNode)                                                                             \
  X(NumberLiteralNode)                                                                             \
  X(ParenthesizedExpressionNode)                                                                   \
  X(BracketExpressionNode)                                                                         \
  X(BlockExpressionNode)                                                                           \
  X(KeyValueEntryNode)                                                                             \
  X(ObjectLiteralNode)                                                                             \
  X(ExpressionStatementNode)                                                                       \
  X(IfStatementNode)                                                                               \
  X(IfExpressionNode)                                                                              \
  X(CatchClauseNode)                                                                               \
  X(TryExpressionNode)                                                                             \
  X(TryStatementNode)                                                                              \
  X(CaseClauseNode)                                                                                \
  X(SwitchExpressionNode)                                                                          \
  X(SwitchStatementNode)                                                                           \
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
  X(OperatorIdentAmpersandNode)                                                                    \
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
  X(OperatorIdentCopyAssignNode)                                                                   \
  X(OperatorIdentMoveAssignNode)                                                                   \
  X(OperatorIdentAsNode)                                                                           \
  X(OperatorFunctionDeclarationNode)                                                               \
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
  X(BitwiseXorAssignStatementNode)                                                                 \
  X(ForInStatementNode)                                                                            \
  X(WhileStatementNode)                                                                            \
  X(LabelStatementNode)                                                                            \
  X(GotoStatementNode)                                                                             \
  X(ContinueStatementNode)                                                                         \
  X(ReturnStatementNode)                                                                           \
  X(FunctionParameterNode)                                                                         \
  X(ImplicitParameterListNode)                                                                     \
  X(FunctionSignatureCaptureAnnotationNode)                                                        \
  X(FunctionSignatureCaptureAnnotationListNode)                                                    \
  X(FunctionSignatureNode)                                                                         \
  X(FunctionBodyNode)                                                                              \
  X(FunctionDeclarationNode)                                                                       \
  X(FunctionExpressionNode)                                                                        \
  X(LambdaExpressionNode)                                                                          \
  X(TypeDeclarationNode)                                                                           \
  X(IntroductoryBindingsNode)                                                                      \
  X(ClassStaticDeclarationNode)                                                                    \
  X(ClassConstDeclarationNode)                                                                     \
  X(ClassFieldNode)                                                                                \
  X(ClassDeclarationNode)                                                                          \
  X(BooleanLiteralNode)                                                                            \
  X(BoolTypeNode)                                                                                  \
  X(ThisLiteralNode)                                                                               \
  X(ClassBaseClassListNode)                                                                        \
  X(ClassConstructorNode)                                                                          \
  X(VisibilityNode)                                                                                \
  X(GenericParameterNode)                                                                          \
  X(GenericParameterListNode)                                                                      \
  X(TypeConstraintNode)                                                                            \
  X(CopyExpressionNode)                                                                            \
  X(MoveExpressionNode)                                                                            \
  X(CopyCtorNameNode)                                                                              \
  X(MoveCtorNameNode)

} // namespace amelia
