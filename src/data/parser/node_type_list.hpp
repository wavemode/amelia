#pragma once

#include <cstdint>

#include "prelude.hpp"

#include "data/lexer/token_type.hpp"
#include "data/sema/type.hpp"
#include "data/source/declaration_visibility.hpp"
#include "data/source/number_literal.hpp"
#include "data/util/integer.hpp"

namespace amelia {

struct ModuleNode {
  List<NodeId> decls;
  List<NodeId> imports;
  List<NodeId> submodules;
};

struct EmptyStmtNode {};

struct IdentifierNode {
  String name;
};

struct StringLiteralNode {
  String contents;
};

struct CharLiteralNode {
  uint32_t code_point;
};

struct NumberLiteralNode {
  NumberLiteral value;
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

struct OperatorIdentIxAssignNode {};

struct OperatorIdentAsNode {
  NodeId type;
};

struct OperatorIdentifierNode {
  NodeId op;
};

struct OperatorFunctionDeclNode {
  NodeId operator_ident;
  NodeId signature;
  Option<NodeId> body;
};

struct LetDeclNode {
  NodeId target;
  Option<NodeId> type;
  Option<NodeId> expr;
};

struct ConstDeclNode {
  NodeId target;
  Option<NodeId> type;
  Option<NodeId> expr;
};

struct PreIncrementStmtNode {
  NodeId target;
};

struct PostIncrementStmtNode {
  NodeId target;
};

struct PreDecrementStmtNode {
  NodeId target;
};

struct PostDecrementStmtNode {
  NodeId target;
};

struct ParenthesizedExprNode {
  List<NodeId> exprs;
};

struct BracketExprNode {
  List<NodeId> exprs;
};

struct BlockExprNode {
  List<NodeId> stmts;
};

struct WithExprNode {
  List<NodeId> args;
  NodeId body;
};

struct KeyValueEntryNode {
  NodeId key;
  NodeId value;
};

struct ExprStmtNode {
  NodeId expr;
};

struct AssignmentStmtNode {
  NodeId target;
  NodeId expr;
};

struct AddAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct SubAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct MulAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct DivAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct ModAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct LeftShiftAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct RightShiftAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct BitwiseAndAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct BitwiseOrAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct BitwiseXorAssignStmtNode {
  NodeId target;
  NodeId expr;
};

struct AnonymousStructLiteralNode {
  List<NodeId> entries;
};

struct AnonymousStructTypeNode {
  List<NodeId> entries;
};

struct IfStmtNode {
  List<NodeId> introductory_decls;
  NodeId condition;
  NodeId then_branch;
  Option<NodeId> else_branch;
};

struct IfExprNode {
  List<NodeId> introductory_decls;
  NodeId condition;
  NodeId then_branch;
  NodeId else_branch;
};

struct CatchClauseNode {
  NodeId exc_type;
  Option<NodeId> var;
  NodeId body;
};

struct TryExprNode {
  NodeId try_block;
  List<NodeId> catch_clauses;
  Option<NodeId> else_branch;
};

struct TryStmtNode {
  NodeId try_block;
  List<NodeId> catch_clauses;
  Option<NodeId> else_branch;
};

struct CaseClauseNode {
  NodeId header;
  NodeId body;
};

struct CaseClauseHeaderNode {
  Option<List<NodeId>> introductory_decls;
  Option<List<NodeId>> exprs;
  Option<NodeId> when_clause;
};

struct WhenClauseNode {
  List<NodeId> introductory_decls;
  NodeId condition;
};

struct SwitchExprNode {
  List<NodeId> introductory_decls;
  NodeId expr;
  List<NodeId> clauses;
  Option<NodeId> default_body;
};

struct SwitchStmtNode {
  List<NodeId> introductory_decls;
  NodeId expr;
  List<NodeId> clauses;
  Option<NodeId> default_body;
};

struct OrExprNode {
  NodeId left;
  NodeId right;
};

struct AndExprNode {
  NodeId left;
  NodeId right;
};

struct BitwiseOrExprNode {
  NodeId left;
  NodeId right;
};

struct BitwiseAndExprNode {
  NodeId left;
  NodeId right;
};

struct BitwiseXorExprNode {
  NodeId left;
  NodeId right;
};

struct EqualsExprNode {
  NodeId left;
  NodeId right;
};

struct NotEqualsExprNode {
  NodeId left;
  NodeId right;
};

struct GreaterEqualsExprNode {
  NodeId left;
  NodeId right;
};

struct LessEqualsExprNode {
  NodeId left;
  NodeId right;
};

struct GreaterExprNode {
  NodeId left;
  NodeId right;
};

struct LessExprNode {
  NodeId left;
  NodeId right;
};

struct LeftShiftExprNode {
  NodeId left;
  NodeId right;
};

struct RightShiftExprNode {
  NodeId left;
  NodeId right;
};

struct AddExprNode {
  NodeId left;
  NodeId right;
};

struct SubtractExprNode {
  NodeId left;
  NodeId right;
};

struct MultiplyExprNode {
  NodeId left;
  NodeId right;
};

struct DivideExprNode {
  NodeId left;
  NodeId right;
};

struct ModuloExprNode {
  NodeId left;
  NodeId right;
};

struct RefExprNode {
  bool is_const;
  bool is_move;
  NodeId expr;
};

struct AwaitExprNode {
  NodeId expr;
};

struct NotExprNode {
  NodeId expr;
};

struct BitwiseNotExprNode {
  NodeId expr;
};

struct DerefExprNode {
  bool is_const;
  NodeId expr;
};

struct PositiveExprNode {
  NodeId expr;
};

struct NegativeExprNode {
  NodeId expr;
};

struct EllipsisExprNode {
  NodeId expr;
};

struct FieldAccessExprNode {
  NodeId object;
  NodeId field;
};

struct NumericFieldAccessExprNode {
  NodeId object;
  Integer field;
};

struct IndexingExprNode {
  NodeId object;
  List<NodeId> indices;
};

struct IndexNode {
  Option<NodeId> name;
  NodeId value;
};

struct FunctionArgumentNode {
  Option<NodeId> name;
  NodeId expr;
};

struct FunctionCallExprNode {
  NodeId callee;
  List<NodeId> args;
};

struct ScopeResolutionExprNode {
  NodeId scope;
  NodeId name;
};

struct BlockStmtNode {
  List<NodeId> stmts;
};

struct ThrowStmtNode {
  Option<NodeId> expr;
};

struct ForInStmtNode {
  List<NodeId> introductory_decls;
  List<NodeId> vars;
  NodeId iterable;
  NodeId body;
};

struct ForInVariableNode {
  NodeId name;
  Option<NodeId> type;
};

struct WhileStmtNode {
  List<NodeId> introductory_decls;
  NodeId condition;
  NodeId body;
};

struct ContinueStmtNode {};

struct LabelStmtNode {
  NodeId label;
};

struct GotoStmtNode {
  NodeId label;
};

struct ReturnStmtNode {
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
  ConstRef,
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
  Option<NodeId> expr;
  Option<List<NodeId>> stmts;
  bool is_default;
  bool is_deleted;
};

struct FunctionDeclNode {
  NodeId name;
  NodeId signature;
  Option<NodeId> body;
};

struct FunctionExprNode {
  NodeId signature;
  Option<NodeId> body;
};

struct LambdaExprNode {
  List<NodeId> parameters;
  NodeId body;
};

struct TypeDeclNode {
  NodeId name;
  Option<NodeId> generic_parameter_list;
  Option<NodeId> type_expr;
};

struct ClassStaticDeclNode {
  NodeId decl;
};

struct ClassConstDeclNode {
  NodeId decl;
};

struct ClassCopyDeclNode {
  NodeId decl;
};

struct ClassMoveDeclNode {
  NodeId decl;
};

struct ClassFieldNode {
  NodeId name;
  Option<NodeId> type;
  Option<NodeId> initializer;
};

struct BaseTypeListNode {
  List<NodeId> base_classes;
};

struct ClassDeclNode {
  NodeId name;
  Option<NodeId> generic_parameter_list;
  Option<NodeId> base_class_list;
  Option<NodeId> header_decls;
  Option<NodeId> implicit_parameter_list;
  Option<NodeId> body;
};

struct ClassHeaderDeclsNode {
  List<NodeId> decls;
};

struct ClassBodyNode {
  List<NodeId> decls;
};

struct ClassHeaderFieldDeclNode {
  Option<NodeId> name;
  NodeId type;
  Option<NodeId> default_value;
};

struct ClassHeaderConstDeclNode {
  NodeId decl;
};

struct ConceptDeclNode {
  NodeId name;
  Option<NodeId> generic_parameter_list;
  Option<NodeId> base_concept_list;
  Option<NodeId> implicit_parameter_list;
  Option<NodeId> body;
};

struct GenericParameterNode {
  bool is_const;
  NodeId name;
  Option<NodeId> constraint;
  Option<NodeId> default_value;
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

struct ThisLiteralNode {};

struct SuperLiteralNode {};

struct ThisTypeNode {};

struct CopyCtorNameNode {};

struct MoveCtorNameNode {};

struct ClassConstructorNode {
  NodeId name;
  NodeId signature;
  Option<NodeId> body;
};

struct ClassDestructorNode {
  NodeId name;
  NodeId signature;
  Option<NodeId> body;
};

struct VisibilityNode {
  DeclarationVisibility visibility;
  Option<NodeId> scope;
  NodeId decl;
};

struct CopyExprNode {
  NodeId expr;
};

struct MoveExprNode {
  NodeId expr;
};

struct ImplicitDeclNode {
  NodeId decl;
};

struct OpenDeclNode {
  NodeId decl;
};

struct OverrideDeclNode {
  NodeId decl;
};

struct DefaultDeclNode {
  NodeId decl;
};

struct DefaultLiteralNode {};

struct AsyncDeclNode {
  NodeId decl;
};

struct ImplTypeExprNode {
  NodeId type;
};

struct AnyTypeExprNode {
  NodeId type;
};

struct ConstTypeExprNode {
  NodeId expr;
};

struct AsyncExprNode {
  NodeId expr;
};

struct BuiltinTypeNode {
  BuiltinKind kind;
};

struct BitIntTypeNode {
  bool is_signed;
};

struct AutoTypeNode {};

struct ImportDeclNode {
  NodeId path;
  Option<List<NodeId>> items;
  Option<NodeId> alias;
};

struct ImportItemNode {
  NodeId name;
  Option<List<NodeId>> sub_items;
  Option<NodeId> alias;
};

struct ImportItemWildcardNode {};

struct ModuleDeclNode {
  NodeId name;
  Option<List<NodeId>> decls;
  Option<List<NodeId>> submodules;
};

struct ExternDeclNode {
  NodeId decl;
};

struct RecordDeclNode {
  NodeId decl;
};

struct UnionDeclNode {
  NodeId name;
  Option<NodeId> generic_parameter_list;
  Option<NodeId> body;
};

struct EnumDeclNode {
  NodeId name;
  Option<NodeId> repr_type;
  Option<NodeId> base_type_list;
  List<NodeId> variants;
};

struct EnumVariantNode {
  NodeId name;
  Option<NodeId> value;
};

struct AnnotationNode {
  NodeId name;
  Option<List<NodeId>> args;
  NodeId stmt;
};

struct TypeOfExprNode {
  NodeId expr;
};

struct BreakStmtNode {};

struct QuestionMarkExprNode {
  NodeId expr;
};

struct ExclamationMarkExprNode {
  NodeId expr;
};

struct SealedDeclNode {
  NodeId decl;
};

struct MutDeclNode {
  NodeId decl;
};

struct InlineDeclNode {
  NodeId decl;
};

struct AbstractDeclNode {
  NodeId decl;
};

#define NODE_TYPE_LIST                                                                             \
  X(ModuleNode)                                                                                    \
  X(IdentifierNode)                                                                                \
  X(EmptyStmtNode)                                                                                 \
  X(LetDeclNode)                                                                                   \
  X(ConstDeclNode)                                                                                 \
  X(StringLiteralNode)                                                                             \
  X(CharLiteralNode)                                                                               \
  X(NumberLiteralNode)                                                                             \
  X(ParenthesizedExprNode)                                                                         \
  X(BracketExprNode)                                                                               \
  X(BlockExprNode)                                                                                 \
  X(WithExprNode)                                                                                  \
  X(KeyValueEntryNode)                                                                             \
  X(AnonymousStructLiteralNode)                                                                    \
  X(AnonymousStructTypeNode)                                                                       \
  X(ExprStmtNode)                                                                                  \
  X(IfStmtNode)                                                                                    \
  X(IfExprNode)                                                                                    \
  X(CatchClauseNode)                                                                               \
  X(TryExprNode)                                                                                   \
  X(TryStmtNode)                                                                                   \
  X(CaseClauseNode)                                                                                \
  X(CaseClauseHeaderNode)                                                                          \
  X(WhenClauseNode)                                                                                \
  X(SwitchExprNode)                                                                                \
  X(SwitchStmtNode)                                                                                \
  X(OrExprNode)                                                                                    \
  X(AndExprNode)                                                                                   \
  X(BitwiseOrExprNode)                                                                             \
  X(BitwiseAndExprNode)                                                                            \
  X(BitwiseXorExprNode)                                                                            \
  X(EqualsExprNode)                                                                                \
  X(NotEqualsExprNode)                                                                             \
  X(GreaterEqualsExprNode)                                                                         \
  X(LessEqualsExprNode)                                                                            \
  X(GreaterExprNode)                                                                               \
  X(LessExprNode)                                                                                  \
  X(LeftShiftExprNode)                                                                             \
  X(RightShiftExprNode)                                                                            \
  X(AddExprNode)                                                                                   \
  X(SubtractExprNode)                                                                              \
  X(MultiplyExprNode)                                                                              \
  X(DivideExprNode)                                                                                \
  X(ModuloExprNode)                                                                                \
  X(RefExprNode)                                                                                   \
  X(AwaitExprNode)                                                                                 \
  X(NotExprNode)                                                                                   \
  X(BitwiseNotExprNode)                                                                            \
  X(DerefExprNode)                                                                                 \
  X(PositiveExprNode)                                                                              \
  X(NegativeExprNode)                                                                              \
  X(EllipsisExprNode)                                                                              \
  X(FieldAccessExprNode)                                                                           \
  X(NumericFieldAccessExprNode)                                                                    \
  X(IndexingExprNode)                                                                              \
  X(IndexNode)                                                                                     \
  X(FunctionArgumentNode)                                                                          \
  X(FunctionCallExprNode)                                                                          \
  X(ScopeResolutionExprNode)                                                                       \
  X(PreIncrementStmtNode)                                                                          \
  X(PostIncrementStmtNode)                                                                         \
  X(PreDecrementStmtNode)                                                                          \
  X(PostDecrementStmtNode)                                                                         \
  X(BlockStmtNode)                                                                                 \
  X(ThrowStmtNode)                                                                                 \
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
  X(OperatorIdentIxAssignNode)                                                                     \
  X(OperatorIdentAsNode)                                                                           \
  X(OperatorFunctionDeclNode)                                                                      \
  X(OperatorIdentifierNode)                                                                        \
  X(AssignmentStmtNode)                                                                            \
  X(AddAssignStmtNode)                                                                             \
  X(SubAssignStmtNode)                                                                             \
  X(MulAssignStmtNode)                                                                             \
  X(DivAssignStmtNode)                                                                             \
  X(ModAssignStmtNode)                                                                             \
  X(LeftShiftAssignStmtNode)                                                                       \
  X(RightShiftAssignStmtNode)                                                                      \
  X(BitwiseAndAssignStmtNode)                                                                      \
  X(BitwiseOrAssignStmtNode)                                                                       \
  X(BitwiseXorAssignStmtNode)                                                                      \
  X(ForInStmtNode)                                                                                 \
  X(ForInVariableNode)                                                                             \
  X(WhileStmtNode)                                                                                 \
  X(LabelStmtNode)                                                                                 \
  X(GotoStmtNode)                                                                                  \
  X(ContinueStmtNode)                                                                              \
  X(ReturnStmtNode)                                                                                \
  X(FunctionParameterNode)                                                                         \
  X(ImplicitParameterListNode)                                                                     \
  X(FunctionSignatureCaptureAnnotationNode)                                                        \
  X(FunctionSignatureCaptureAnnotationListNode)                                                    \
  X(FunctionSignatureNode)                                                                         \
  X(FunctionBodyNode)                                                                              \
  X(FunctionDeclNode)                                                                              \
  X(FunctionExprNode)                                                                              \
  X(LambdaExprNode)                                                                                \
  X(TypeDeclNode)                                                                                  \
  X(ClassStaticDeclNode)                                                                           \
  X(ClassConstDeclNode)                                                                            \
  X(ClassCopyDeclNode)                                                                             \
  X(ClassMoveDeclNode)                                                                             \
  X(ClassFieldNode)                                                                                \
  X(ClassDeclNode)                                                                                 \
  X(ConceptDeclNode)                                                                               \
  X(ClassHeaderDeclsNode)                                                                          \
  X(ClassBodyNode)                                                                                 \
  X(ClassHeaderFieldDeclNode)                                                                      \
  X(ClassHeaderConstDeclNode)                                                                      \
  X(BooleanLiteralNode)                                                                            \
  X(ThisLiteralNode)                                                                               \
  X(SuperLiteralNode)                                                                              \
  X(ThisTypeNode)                                                                                  \
  X(BaseTypeListNode)                                                                              \
  X(ClassConstructorNode)                                                                          \
  X(ClassDestructorNode)                                                                           \
  X(VisibilityNode)                                                                                \
  X(GenericParameterNode)                                                                          \
  X(GenericParameterListNode)                                                                      \
  X(TypeConstraintNode)                                                                            \
  X(ImplTypeExprNode)                                                                              \
  X(AnyTypeExprNode)                                                                               \
  X(ConstTypeExprNode)                                                                             \
  X(AsyncExprNode)                                                                                 \
  X(CopyExprNode)                                                                                  \
  X(MoveExprNode)                                                                                  \
  X(CopyCtorNameNode)                                                                              \
  X(MoveCtorNameNode)                                                                              \
  X(ImplicitDeclNode)                                                                              \
  X(OpenDeclNode)                                                                                  \
  X(OverrideDeclNode)                                                                              \
  X(DefaultDeclNode)                                                                               \
  X(DefaultLiteralNode)                                                                            \
  X(BuiltinTypeNode)                                                                               \
  X(BitIntTypeNode)                                                                                \
  X(AutoTypeNode)                                                                                  \
  X(ImportDeclNode)                                                                                \
  X(ImportItemNode)                                                                                \
  X(ImportItemWildcardNode)                                                                        \
  X(ModuleDeclNode)                                                                                \
  X(AsyncDeclNode)                                                                                 \
  X(ExternDeclNode)                                                                                \
  X(RecordDeclNode)                                                                                \
  X(UnionDeclNode)                                                                                 \
  X(EnumDeclNode)                                                                                  \
  X(EnumVariantNode)                                                                               \
  X(BreakStmtNode)                                                                                 \
  X(AnnotationNode)                                                                                \
  X(TypeOfExprNode)                                                                                \
  X(QuestionMarkExprNode)                                                                          \
  X(ExclamationMarkExprNode)                                                                       \
  X(SealedDeclNode)                                                                                \
  X(MutDeclNode)                                                                                   \
  X(InlineDeclNode)                                                                                \
  X(AbstractDeclNode)

} // namespace amelia
