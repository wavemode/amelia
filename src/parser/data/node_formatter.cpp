#include "node_formatter.hpp"

#include "lexer/data/lexer.hpp"
#include "parser/data/node.hpp"
#include "parser/interfaces/abstract_node_repository.hpp"
#include "util/data/integer.hpp"
#include "util/data/serialize.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

NodeFormatter::NodeFormatter(const AbstractNodeRepository &node_repo)
    : m_node_repo(node_repo), m_fields_printed(0), m_current_indent(0) {}

void NodeFormatter::format_node(AbstractString &out, NodeId node_id) {
  const Node &node = m_node_repo.get_node(node_id);
  node_type_to_string(out, node.type());
  out.append('(');
  uint32_t previous_fields_printed = m_fields_printed;
  m_fields_printed = 0;
  m_current_indent += 2;
  switch (node.type()) {
  case NodeType::IdentifierNode: {
    const auto &n = node.as_IdentifierNode();
    print_boolean_field(out, "raw", n.raw);
    String s;
    Serialize::literal(n.contents).quoted().to_string(s);
    print_text_field(out, "contents", s.text());
    break;
  }
  case NodeType::IdentifierPatternNode: {
    const auto &n = node.as_IdentifierPatternNode();
    print_node_field(out, "identifier", n.identifier);
    print_node_field(out, "type_annotation", n.type_annotation);
    break;
  }
  case NodeType::ArrayPatternNode: {
    const auto &n = node.as_ArrayPatternNode();
    print_node_field(out, "elements", n.elements);
    break;
  }
  case NodeType::TuplePatternNode: {
    const auto &n = node.as_TuplePatternNode();
    print_node_field(out, "elements", n.elements);
    break;
  }
  case NodeType::StructFieldPatternNode: {
    const auto &n = node.as_StructFieldPatternNode();
    print_node_field(out, "pattern", n.pattern);
    print_node_field(out, "field", n.field);
    break;
  }
  case NodeType::StructPatternNode: {
    const auto &n = node.as_StructPatternNode();
    print_node_field(out, "elements", n.elements);
    break;
  }
  case NodeType::InitializerNode: {
    const auto &n = node.as_InitializerNode();
    print_node_field(out, "pattern", n.pattern);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::LetDeclNode: {
    const auto &n = node.as_LetDeclNode();
    print_node_field(out, "inits", n.inits);
    break;
  }
  case NodeType::ConstDeclNode: {
    const auto &n = node.as_ConstDeclNode();
    print_node_field(out, "inits", n.inits);
    break;
  }
  case NodeType::StringLiteralNode: {
    const auto &n = node.as_StringLiteralNode();
    print_boolean_field(out, "raw", n.raw);
    String s;
    Serialize::literal(n.contents).quoted().to_string(s);
    print_text_field(out, "contents", s.text());
    break;
  }
  case NodeType::StringLiteralSeriesNode: {
    const auto &n = node.as_StringLiteralSeriesNode();
    print_node_field(out, "literals", n.literals);
    break;
  }
  case NodeType::CharLiteralNode: {
    const auto &n = node.as_CharLiteralNode();
    String s;
    Serialize::literal(n.contents).quoted().to_string(s);
    print_text_field(out, "contents", s.text());
    out.append('\'');
    break;
  }
  case NodeType::NumberLiteralNode: {
    const auto &n = node.as_NumberLiteralNode();
    String s;
    Serialize::literal(n.contents).quoted().to_string(s);
    print_text_field(out, "contents", s.text());
    break;
  }
  case NodeType::ParenthesizedExprNode: {
    const auto &n = node.as_ParenthesizedExprNode();
    print_node_field(out, "exprs", n.exprs);
    break;
  }
  case NodeType::ArrayExprNode: {
    const auto &n = node.as_ArrayExprNode();
    print_node_field(out, "size", n.size);
    if (n.infer_size) {
      print_boolean_field(out, "infer_size", n.infer_size);
    }
    if (n.is_const) {
      print_boolean_field(out, "is_const", n.is_const);
    }
    print_node_field(out, "type_expr", n.type_expr);
    print_node_field(out, "elements", n.elements);
    break;
  }
  case NodeType::ModuleNode: {
    const auto &n = node.as_ModuleNode();
    print_node_field(out, "decls", n.decls);
    print_node_field(out, "imports", n.imports);
    print_node_field(out, "submodules", n.submodules);
    break;
  }
  case NodeType::BlockExprNode: {
    const auto &n = node.as_BlockExprNode();
    print_node_field(out, "stmts", n.stmts);
    break;
  }
  case NodeType::WithExprNode: {
    const auto &n = node.as_WithExprNode();
    print_node_field(out, "args", n.args);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::WithAssignmentNode: {
    const auto &n = node.as_WithAssignmentNode();
    print_node_field(out, "var", n.var);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::WithStmtNode: {
    const auto &n = node.as_WithStmtNode();
    print_node_field(out, "args", n.args);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::AnonymousStructLiteralNode: {
    const auto &n = node.as_AnonymousStructLiteralNode();
    print_node_field(out, "entries", n.entries);
    break;
  }
  case NodeType::AnonymousStructTypeNode: {
    const auto &n = node.as_AnonymousStructTypeNode();
    print_node_field(out, "entries", n.entries);
    break;
  }
  case NodeType::KeyValueEntryNode: {
    const auto &n = node.as_KeyValueEntryNode();
    print_node_field(out, "key", n.key);
    print_node_field(out, "value", n.value);
    break;
  }
  case NodeType::ExprStmtNode: {
    const auto &n = node.as_ExprStmtNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::IfStmtNode: {
    const auto &n = node.as_IfStmtNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "condition", n.condition);
    print_node_field(out, "then_branch", n.then_branch);
    print_node_field(out, "else_branch", n.else_branch);
    break;
  }
  case NodeType::IfExprNode: {
    const auto &n = node.as_IfExprNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "condition", n.condition);
    print_node_field(out, "then_branch", n.then_branch);
    print_node_field(out, "else_branch", n.else_branch);
    break;
  }
  case NodeType::CatchClauseNode: {
    const auto &n = node.as_CatchClauseNode();
    print_node_field(out, "exc_type", n.exc_type);
    print_node_field(out, "var", n.var);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::TryExprNode: {
    const auto &n = node.as_TryExprNode();
    print_node_field(out, "try_block", n.try_block);
    print_node_field(out, "catch_clauses", n.catch_clauses);
    print_node_field(out, "else_branch", n.else_branch);
    break;
  }
  case NodeType::TryStmtNode: {
    const auto &n = node.as_TryStmtNode();
    print_node_field(out, "try_block", n.try_block);
    print_node_field(out, "catch_clauses", n.catch_clauses);
    print_node_field(out, "else_branch", n.else_branch);
    break;
  }
  case NodeType::WhenClauseItemNode: {
    const auto &n = node.as_WhenClauseItemNode();
    print_boolean_field(out, "is", n.is);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::WhenClauseNode: {
    const auto &n = node.as_WhenClauseNode();
    print_node_field(out, "items", n.items);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::WhenExprNode: {
    const auto &n = node.as_WhenExprNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "expr", n.expr);
    print_node_field(out, "clauses", n.clauses);
    print_node_field(out, "else_expr", n.else_expr);
    break;
  }
  case NodeType::WhenStmtNode: {
    const auto &n = node.as_WhenStmtNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "expr", n.expr);
    print_node_field(out, "clauses", n.clauses);
    print_node_field(out, "else_stmt", n.else_stmt);
    break;
  }
  case NodeType::StaticExprNode: {
    const auto &n = node.as_StaticExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::StaticStmtNode: {
    const auto &n = node.as_StaticStmtNode();
    print_node_field(out, "stmt", n.stmt);
    break;
  }
  case NodeType::OrExprNode: {
    const auto &n = node.as_OrExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::AndExprNode: {
    const auto &n = node.as_AndExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::BitwiseOrExprNode: {
    const auto &n = node.as_BitwiseOrExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::BitwiseXorExprNode: {
    const auto &n = node.as_BitwiseXorExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::BitwiseAndExprNode: {
    const auto &n = node.as_BitwiseAndExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::EqualsExprNode: {
    const auto &n = node.as_EqualsExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::NotEqualsExprNode: {
    const auto &n = node.as_NotEqualsExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::GreaterEqualsExprNode: {
    const auto &n = node.as_GreaterEqualsExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::LessEqualsExprNode: {
    const auto &n = node.as_LessEqualsExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::GreaterExprNode: {
    const auto &n = node.as_GreaterExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::LessExprNode: {
    const auto &n = node.as_LessExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::LeftShiftExprNode: {
    const auto &n = node.as_LeftShiftExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::RightShiftExprNode: {
    const auto &n = node.as_RightShiftExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::AddExprNode: {
    const auto &n = node.as_AddExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::SubtractExprNode: {
    const auto &n = node.as_SubtractExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::MultiplyExprNode: {
    const auto &n = node.as_MultiplyExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::DivideExprNode: {
    const auto &n = node.as_DivideExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::ModuloExprNode: {
    const auto &n = node.as_ModuloExprNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::RefExprNode: {
    const auto &n = node.as_RefExprNode();
    print_boolean_field(out, "is_const", n.is_const);
    print_boolean_field(out, "is_move", n.is_move);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::AwaitExprNode: {
    const auto &n = node.as_AwaitExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::NotExprNode: {
    const auto &n = node.as_NotExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::BitwiseNotExprNode: {
    const auto &n = node.as_BitwiseNotExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::DerefExprNode: {
    const auto &n = node.as_DerefExprNode();
    print_boolean_field(out, "is_const", n.is_const);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::PositiveExprNode: {
    const auto &n = node.as_PositiveExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::NegateExprNode: {
    const auto &n = node.as_NegateExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::EllipsisExprNode: {
    const auto &n = node.as_EllipsisExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::DollarSignExprNode: {
    const auto &n = node.as_DollarSignExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::FieldAccessExprNode: {
    const auto &n = node.as_FieldAccessExprNode();
    print_node_field(out, "object", n.object);
    print_node_field(out, "field", n.field);
    break;
  }
  case NodeType::NumericFieldAccessExprNode: {
    const auto &n = node.as_NumericFieldAccessExprNode();
    print_node_field(out, "object", n.object);
    open_line(out);
    out.append("field=\"");
    out.append(n.field);
    out.append('"');
    break;
  }
  case NodeType::AsExprNode: {
    const auto &n = node.as_AsExprNode();
    print_node_field(out, "expr", n.expr);
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::IsExprNode: {
    const auto &n = node.as_IsExprNode();
    print_node_field(out, "expr", n.expr);
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::IndexingExprNode: {
    const auto &n = node.as_IndexingExprNode();
    print_node_field(out, "object", n.object);
    print_node_field(out, "indices", n.indices);
    break;
  }
  case NodeType::IndexNode: {
    const auto &n = node.as_IndexNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "value", n.value);
    break;
  }
  case NodeType::FunctionArgumentNode: {
    const auto &n = node.as_FunctionArgumentNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::FunctionCallExprNode: {
    const auto &n = node.as_FunctionCallExprNode();
    print_node_field(out, "callee", n.callee);
    print_node_field(out, "args", n.args);
    break;
  }
  case NodeType::ScopeResolutionExprNode: {
    const auto &n = node.as_ScopeResolutionExprNode();
    print_node_field(out, "scope", n.scope);
    print_node_field(out, "name", n.name);
    break;
  }
  case NodeType::PreIncrementStmtNode: {
    const auto &n = node.as_PreIncrementStmtNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::PostIncrementStmtNode: {
    const auto &n = node.as_PostIncrementStmtNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::PreDecrementStmtNode: {
    const auto &n = node.as_PreDecrementStmtNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::PostDecrementStmtNode: {
    const auto &n = node.as_PostDecrementStmtNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::ThrowStmtNode: {
    const auto &n = node.as_ThrowStmtNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::EmptyStmtNode:
  case NodeType::UnderscorePatternNode:
  case NodeType::OperatorIdentAddNode:
  case NodeType::OperatorIdentSubNode:
  case NodeType::OperatorIdentStarNode:
  case NodeType::OperatorIdentDivNode:
  case NodeType::OperatorIdentModNode:
  case NodeType::OperatorIdentIncNode:
  case NodeType::OperatorIdentDecNode:
  case NodeType::OperatorIdentEqNode:
  case NodeType::OperatorIdentNeqNode:
  case NodeType::OperatorIdentGtNode:
  case NodeType::OperatorIdentLtNode:
  case NodeType::OperatorIdentGteNode:
  case NodeType::OperatorIdentLteNode:
  case NodeType::OperatorIdentNotNode:
  case NodeType::OperatorIdentAndNode:
  case NodeType::OperatorIdentOrNode:
  case NodeType::OperatorIdentBitwiseNotNode:
  case NodeType::OperatorIdentAmpersandNode:
  case NodeType::OperatorIdentBitwiseOrNode:
  case NodeType::OperatorIdentBitwiseXorNode:
  case NodeType::OperatorIdentLeftShiftNode:
  case NodeType::OperatorIdentRightShiftNode:
  case NodeType::OperatorIdentAssignNode:
  case NodeType::OperatorIdentAddAssignNode:
  case NodeType::OperatorIdentSubAssignNode:
  case NodeType::OperatorIdentMulAssignNode:
  case NodeType::OperatorIdentDivAssignNode:
  case NodeType::OperatorIdentModAssignNode:
  case NodeType::OperatorIdentBitwiseAndAssignNode:
  case NodeType::OperatorIdentBitwiseOrAssignNode:
  case NodeType::OperatorIdentBitwiseXorAssignNode:
  case NodeType::OperatorIdentLeftShiftAssignNode:
  case NodeType::OperatorIdentRightShiftAssignNode:
  case NodeType::OperatorIdentIxNode:
  case NodeType::OperatorIdentFuncallNode:
  case NodeType::OperatorIdentIxAssignNode:
  case NodeType::ThisLiteralNode:
  case NodeType::SuperLiteralNode:
  case NodeType::ThisTypeNode:
    break;
  case NodeType::OperatorIdentAsNode: {
    const auto &n = node.as_OperatorIdentAsNode();
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::OperatorIdentifierNode: {
    const auto &n = node.as_OperatorIdentifierNode();
    print_node_field(out, "op", n.op);
    break;
  }
  case NodeType::OperatorFunctionDeclNode: {
    const auto &n = node.as_OperatorFunctionDeclNode();
    print_node_field(out, "operator_ident", n.operator_ident);
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::AssignmentStmtNode: {
    const auto &n = node.as_AssignmentStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::AddAssignStmtNode: {
    const auto &n = node.as_AddAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::SubAssignStmtNode: {
    const auto &n = node.as_SubAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::MulAssignStmtNode: {
    const auto &n = node.as_MulAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::DivAssignStmtNode: {
    const auto &n = node.as_DivAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::ModAssignStmtNode: {
    const auto &n = node.as_ModAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::LeftShiftAssignStmtNode: {
    const auto &n = node.as_LeftShiftAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::RightShiftAssignStmtNode: {
    const auto &n = node.as_RightShiftAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::BitwiseAndAssignStmtNode: {
    const auto &n = node.as_BitwiseAndAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::BitwiseOrAssignStmtNode: {
    const auto &n = node.as_BitwiseOrAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::BitwiseXorAssignStmtNode: {
    const auto &n = node.as_BitwiseXorAssignStmtNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::ForInStmtNode: {
    const auto &n = node.as_ForInStmtNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "pattern", n.pattern);
    print_node_field(out, "iterable", n.iterable);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::WhileStmtNode: {
    const auto &n = node.as_WhileStmtNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "condition", n.condition);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::LabelStmtNode: {
    const auto &n = node.as_LabelStmtNode();
    print_node_field(out, "label", n.label);
    break;
  }
  case NodeType::GotoStmtNode: {
    const auto &n = node.as_GotoStmtNode();
    print_node_field(out, "label", n.label);
    break;
  }
  case NodeType::ContinueStmtNode: {
    break;
  }
  case NodeType::ReturnStmtNode: {
    const auto &n = node.as_ReturnStmtNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::FunctionParameterNode: {
    const auto &n = node.as_FunctionParameterNode();
    print_boolean_field(out, "variadic", n.variadic);
    print_node_field(out, "name", n.name);
    print_node_field(out, "type", n.type);
    print_node_field(out, "default_value", n.default_value);
    break;
  }
  case NodeType::FunctionSignatureNode: {
    const auto &n = node.as_FunctionSignatureNode();
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "parameters", n.parameters);
    print_node_field(out, "implicit_parameter_list", n.implicit_parameter_list);
    print_node_field(out, "capture_list", n.capture_list);
    print_node_field(out, "return_type", n.return_type);
    break;
  }
  case NodeType::FunctionBodyNode: {
    const auto &n = node.as_FunctionBodyNode();
    print_node_field(out, "expr", n.expr);
    print_node_field(out, "stmts", n.stmts);
    print_boolean_field(out, "is_default", n.is_default);
    print_boolean_field(out, "is_deleted", n.is_deleted);
    break;
  }
  case NodeType::ImplicitParameterListNode: {
    const auto &n = node.as_ImplicitParameterListNode();
    print_node_field(out, "parameters", n.parameters);
    break;
  }
  case NodeType::FunctionSignatureCaptureDesignatorListNode: {
    const auto &n = node.as_FunctionSignatureCaptureDesignatorListNode();
    print_node_field(out, "captures", n.captures);
    break;
  }
  case NodeType::FunctionSignatureCaptureDesignatorNode: {
    const auto &n = node.as_FunctionSignatureCaptureDesignatorNode();
    if (n.kind.has_value()) {
      Text kind;
      switch (n.kind.value()) {
      case FunctionCaptureKind::Copy:
        kind = "copy";
        break;
      case FunctionCaptureKind::Move:
        kind = "move";
        break;
      case FunctionCaptureKind::Ref:
        kind = "ref";
        break;
      case FunctionCaptureKind::ConstRef:
        kind = "const_ref";
        break;
      case FunctionCaptureKind::MoveRef:
        kind = "move_ref";
        break;
      }
      print_text_field(out, "kind", kind);
    }
    print_node_field(out, "var", n.var);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::FunctionDeclNode: {
    const auto &n = node.as_FunctionDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::FunctionExprNode: {
    const auto &n = node.as_FunctionExprNode();
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::LambdaExprNode: {
    const auto &n = node.as_LambdaExprNode();
    print_node_field(out, "parameters", n.parameters);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::TypeDeclNode: {
    const auto &n = node.as_TypeDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "type_expr", n.type_expr);
    break;
  }
  case NodeType::ClassStaticDeclNode: {
    const auto &n = node.as_ClassStaticDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ClassConstDeclNode: {
    const auto &n = node.as_ClassConstDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ClassFieldNode: {
    const auto &n = node.as_ClassFieldNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "type", n.type);
    print_node_field(out, "initializer", n.initializer);
    break;
  }
  case NodeType::BaseTypeListNode: {
    const auto &n = node.as_BaseTypeListNode();
    print_node_field(out, "base_classes", n.base_classes);
    break;
  }
  case NodeType::ClassDeclNode: {
    const auto &n = node.as_ClassDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "base_class_list", n.base_class_list);
    print_node_field(out, "header_decls", n.header_decls);
    print_node_field(out, "implicit_parameter_list", n.implicit_parameter_list);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::ConceptDeclNode: {
    const auto &n = node.as_ConceptDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "base_concept_list", n.base_concept_list);
    print_node_field(out, "implicit_parameter_list", n.implicit_parameter_list);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::ClassHeaderDeclsNode: {
    const auto &n = node.as_ClassHeaderDeclsNode();
    print_node_field(out, "decls", n.decls);
    break;
  }
  case NodeType::ClassBodyNode: {
    const auto &n = node.as_ClassBodyNode();
    print_node_field(out, "decls", n.decls);
    break;
  }
  case NodeType::ClassHeaderConstDeclNode: {
    const auto &n = node.as_ClassHeaderConstDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ClassHeaderFieldDeclNode: {
    const auto &n = node.as_ClassHeaderFieldDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "type", n.type);
    print_node_field(out, "default_value", n.default_value);
    break;
  }
  case NodeType::GenericParameterNode: {
    const auto &n = node.as_GenericParameterNode();
    print_boolean_field(out, "is_const", n.is_const);
    print_node_field(out, "name", n.name);
    print_node_field(out, "constraint", n.constraint);
    print_node_field(out, "default_value", n.default_value);
    break;
  }
  case NodeType::TypeConstraintNode: {
    const auto &n = node.as_TypeConstraintNode();
    print_node_field(out, "type", n.type);
    print_node_field(out, "constraint", n.constraint);
    break;
  }
  case NodeType::ImplTypeExprNode: {
    const auto &n = node.as_ImplTypeExprNode();
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::AnyTypeExprNode: {
    const auto &n = node.as_AnyTypeExprNode();
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::ConstTypeExprNode: {
    const auto &n = node.as_ConstTypeExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::AsyncExprNode: {
    const auto &n = node.as_AsyncExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::GenericParameterListNode: {
    const auto &n = node.as_GenericParameterListNode();
    print_node_field(out, "parameters", n.parameters);
    print_node_field(out, "additional_constraints", n.additional_constraints);
    break;
  }
  case NodeType::BooleanLiteralNode: {
    const auto &n = node.as_BooleanLiteralNode();
    print_boolean_field(out, "value", n.value);
    break;
  }
  case NodeType::ClassConstructorNode: {
    const auto &n = node.as_ClassConstructorNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::ClassDestructorNode: {
    const auto &n = node.as_ClassDestructorNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::VisibilityNode: {
    const auto &n = node.as_VisibilityNode();
    Text visibility;
    switch (n.visibility) {
    case DeclarationVisibility::Public:
      visibility = "public";
      break;
    case DeclarationVisibility::Private:
      visibility = "private";
      break;
    case DeclarationVisibility::Protected:
      visibility = "protected";
      break;
    case DeclarationVisibility::Local:
      visibility = "local";
      break;
    case DeclarationVisibility::Default:
      visibility = "default";
      break;
    }
    print_text_field(out, "visibility", visibility);
    print_node_field(out, "scope", n.scope);
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::CopyExprNode: {
    const auto &n = node.as_CopyExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::MoveExprNode: {
    const auto &n = node.as_MoveExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::ImplicitDeclNode: {
    const auto &n = node.as_ImplicitDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::ClassBodyImplicitDeclNode: {
    const auto &n = node.as_ClassBodyImplicitDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::OpenDeclNode: {
    const auto &n = node.as_OpenDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::OverrideDeclNode: {
    const auto &n = node.as_OverrideDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::DefaultDeclNode: {
    const auto &n = node.as_DefaultDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::DefaultLiteralNode: {
    break;
  }
  case NodeType::BuiltinTypeNode: {
    const auto &n = node.as_BuiltinTypeNode();
    open_line(out);
    out.append("kind=");
    serialize_builtin_kind(n.kind).to_string(out);
    break;
  }
  case NodeType::BitIntTypeNode: {
    const auto &n = node.as_BitIntTypeNode();
    print_boolean_field(out, "is_signed", n.is_signed);
    break;
  }
  case NodeType::ImportDeclNode: {
    const auto &n = node.as_ImportDeclNode();
    print_node_field(out, "path", n.path);
    print_node_field(out, "items", n.items);
    print_node_field(out, "alias", n.alias);
    break;
  }
  case NodeType::ImportItemNode: {
    const auto &n = node.as_ImportItemNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "sub_items", n.sub_items);
    print_node_field(out, "alias", n.alias);
    break;
  }
  case NodeType::ImportItemWildcardNode: {
    break;
  }
  case NodeType::ModuleDeclNode: {
    const auto &n = node.as_ModuleDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "decls", n.decls);
    print_node_field(out, "submodules", n.submodules);
    break;
  }
  case NodeType::AsyncDeclNode: {
    const auto &n = node.as_AsyncDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::RecordDeclNode: {
    const auto &n = node.as_RecordDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::UnionDeclNode: {
    const auto &n = node.as_UnionDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::EnumDeclNode: {
    const auto &n = node.as_EnumDeclNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "repr_type", n.repr_type);
    print_node_field(out, "base_type_list", n.base_type_list);
    print_node_field(out, "variants", n.variants);
    break;
  }
  case NodeType::EnumVariantNode: {
    const auto &n = node.as_EnumVariantNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "value", n.value);
    break;
  }
  case NodeType::BreakStmtNode: {
    break;
  }
  case NodeType::AnnotationNode: {
    const auto &n = node.as_AnnotationNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "args", n.args);
    print_node_field(out, "stmt", n.stmt);
    break;
  }
  case NodeType::TypeOfExprNode: {
    const auto &n = node.as_TypeOfExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::QuestionMarkExprNode: {
    const auto &n = node.as_QuestionMarkExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::ExclamationMarkExprNode: {
    const auto &n = node.as_ExclamationMarkExprNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::SealedDeclNode: {
    const auto &n = node.as_SealedDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::MutDeclNode: {
    const auto &n = node.as_MutDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::AbstractDeclNode: {
    const auto &n = node.as_AbstractDeclNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::TypeFieldNode: {
    break;
  }
  }
  m_current_indent -= 2;
  if (m_fields_printed > 0) {
    open_line(out, false);
  }
  out.append(')');
  m_fields_printed = previous_fields_printed;
}

void NodeFormatter::print_node_field(AbstractString &out, Text name,
                                     const List<NodeId> &nodes_value) {
  if (nodes_value.size() == 0)
    return;
  open_line(out);
  out.append(name);
  out.append("=[");
  const uint32_t old_fields_printed = m_fields_printed;
  m_fields_printed = 0;
  m_current_indent += 2;
  for (size_t i = 0; i < nodes_value.size(); ++i) {
    open_line(out);
    format_node(out, nodes_value[i]);
  }
  m_current_indent -= 2;
  if (m_fields_printed > 0) {
    open_line(out, false);
  }
  out.append(']');
  m_fields_printed = old_fields_printed;
}

void NodeFormatter::print_node_field(AbstractString &out, Text name,
                                     const Option<List<NodeId>> &nodes) {
  if (!nodes.has_value() || nodes.value().size() == 0)
    return;
  open_line(out);
  out.append(name);
  out.append("=[");
  const auto &nodes_value = nodes.value();
  const uint32_t old_fields_printed = m_fields_printed;
  m_fields_printed = 0;
  m_current_indent += 2;
  for (size_t i = 0; i < nodes_value.size(); ++i) {
    open_line(out);
    format_node(out, nodes_value[i]);
  }
  m_current_indent -= 2;
  if (m_fields_printed > 0) {
    open_line(out, false);
  }
  out.append(']');
  m_fields_printed = old_fields_printed;
}

void NodeFormatter::print_node_field(AbstractString &out, Text name,
                                     Option<NodeId> node_id) {
  if (!node_id.has_value()) {
    return;
  }
  open_line(out);
  out.append(name);
  out.append('=');
  format_node(out, node_id.value());
}
void NodeFormatter::print_text_field(AbstractString &out, Text name,
                                     Text value) {
  open_line(out);
  out.append(name);
  out.append('=');
  out.append(value);
}

void NodeFormatter::print_numeric_field(AbstractString &out, Text name,
                                        const Integer &value) {
  open_line(out);
  out.append(name);
  out.append('=');
  value.to_string(out);
}

void NodeFormatter::print_boolean_field(AbstractString &out, Text name,
                                        bool value) {
  open_line(out);
  out.append(name);
  out.append('=');
  TextUtils::to_string(out, value);
}

void NodeFormatter::open_line(AbstractString &out, bool with_comma) {
  if (with_comma && m_fields_printed > 0) {
    out.append(',');
  }
  out.append('\n');
  print_indent(out);
  ++m_fields_printed;
}

void NodeFormatter::print_indent(AbstractString &out) const {
  for (uint32_t i = 0; i < m_current_indent; ++i) {
    out.append(' ');
  }
}

} // namespace amelia
