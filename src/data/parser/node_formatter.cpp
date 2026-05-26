#include "node_formatter.h"

#include "data/lexer/lexer.h"

namespace amelia {

NodeFormatter::NodeFormatter(
    const AbstractNodeRepository &node_repo, const AbstractTokenRepository &token_repo
)
    : m_token_repo(token_repo), m_token_formatter(TokenFormatter(token_repo)),
      m_node_repo(node_repo), m_fields_printed(0), m_current_indent(0) {}

void NodeFormatter::format_node(AbstractString &out, NodeId node_id) {
  const Node &node = m_node_repo.get_node(node_id);
  node_type_to_string(out, node.type());
  out.append('(');
  int previous_fields_printed = m_fields_printed;
  m_fields_printed = 0;
  m_current_indent += 2;
  switch (node.type()) {
  case NodeType::IdentifierNode: {
    const auto &n = node.as_IdentifierNode();
    print_token_field(out, "token", n.token);
    break;
  }
  case NodeType::EmptyStatementNode: {
    break;
  }
  case NodeType::LetDeclarationNode: {
    const auto &n = node.as_LetDeclarationNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "type", n.type);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::ConstDeclarationNode: {
    const auto &n = node.as_ConstDeclarationNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "type", n.type);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::StringLiteralNode: {
    const auto &n = node.as_StringLiteralNode();
    open_line(out);
    out.append("lit=\"");
    Lexer::read_string_literal(out, m_token_repo.get_token(n.lit).contents, true);
    out.append('"');
    break;
  }
  case NodeType::CharLiteralNode: {
    const auto &n = node.as_CharLiteralNode();
    open_line(out);
    out.append("lit='");
    Lexer::read_char_literal(out, m_token_repo.get_token(n.lit).contents, true);
    out.append('\'');
    break;
  }
  case NodeType::NumberLiteralNode: {
    const auto &n = node.as_NumberLiteralNode();
    print_token_field(out, "lit", n.lit);
    break;
  }
  case NodeType::ParenthesizedExpressionNode: {
    const auto &n = node.as_ParenthesizedExpressionNode();
    print_node_field(out, "exprs", n.exprs);
    break;
  }
  case NodeType::ModuleNode: {
    const auto &n = node.as_ModuleNode();
    print_node_field(out, "decls", n.decls);
    break;
  }
  case NodeType::BracketExpressionNode: {
    const auto &n = node.as_BracketExpressionNode();
    print_node_field(out, "exprs", n.exprs);
    break;
  }
  case NodeType::BlockExpressionNode: {
    const auto &n = node.as_BlockExpressionNode();
    print_node_field(out, "stmts", n.stmts);
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
  case NodeType::ExpressionStatementNode: {
    const auto &n = node.as_ExpressionStatementNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::IfStatementNode: {
    const auto &n = node.as_IfStatementNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "condition", n.condition);
    print_node_field(out, "then_branch", n.then_branch);
    print_node_field(out, "else_branch", n.else_branch);
    break;
  }
  case NodeType::IfExpressionNode: {
    const auto &n = node.as_IfExpressionNode();
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
  case NodeType::TryExpressionNode: {
    const auto &n = node.as_TryExpressionNode();
    print_node_field(out, "try_block", n.try_block);
    print_node_field(out, "clauses", n.clauses);
    break;
  }
  case NodeType::TryStatementNode: {
    const auto &n = node.as_TryStatementNode();
    print_node_field(out, "try_block", n.try_block);
    print_node_field(out, "clauses", n.clauses);
    break;
  }
  case NodeType::CaseClauseNode: {
    const auto &n = node.as_CaseClauseNode();
    print_node_field(out, "header", n.header);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::CaseClauseHeaderNode: {
    const auto &n = node.as_CaseClauseHeaderNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "exprs", n.exprs);
    print_node_field(out, "when_clause", n.when_clause);
    break;
  }
  case NodeType::WhenClauseNode: {
    const auto &n = node.as_WhenClauseNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "condition", n.condition);
    break;
  }
  case NodeType::SwitchExpressionNode: {
    const auto &n = node.as_SwitchExpressionNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "expr", n.expr);
    print_node_field(out, "clauses", n.clauses);
    print_node_field(out, "default_body", n.default_body);
    break;
  }
  case NodeType::SwitchStatementNode: {
    const auto &n = node.as_SwitchStatementNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "expr", n.expr);
    print_node_field(out, "clauses", n.clauses);
    print_node_field(out, "default_body", n.default_body);
    break;
  }
  case NodeType::OrExpressionNode: {
    const auto &n = node.as_OrExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::AndExpressionNode: {
    const auto &n = node.as_AndExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::BitwiseOrExpressionNode: {
    const auto &n = node.as_BitwiseOrExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::BitwiseXorExpressionNode: {
    const auto &n = node.as_BitwiseXorExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::BitwiseAndExpressionNode: {
    const auto &n = node.as_BitwiseAndExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::EqualsExpressionNode: {
    const auto &n = node.as_EqualsExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::NotEqualsExpressionNode: {
    const auto &n = node.as_NotEqualsExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::GreaterEqualsExpressionNode: {
    const auto &n = node.as_GreaterEqualsExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::LessEqualsExpressionNode: {
    const auto &n = node.as_LessEqualsExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::GreaterExpressionNode: {
    const auto &n = node.as_GreaterExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::LessExpressionNode: {
    const auto &n = node.as_LessExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::LeftShiftExpressionNode: {
    const auto &n = node.as_LeftShiftExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::RightShiftExpressionNode: {
    const auto &n = node.as_RightShiftExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::AddExpressionNode: {
    const auto &n = node.as_AddExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::SubtractExpressionNode: {
    const auto &n = node.as_SubtractExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::MultiplyExpressionNode: {
    const auto &n = node.as_MultiplyExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::DivideExpressionNode: {
    const auto &n = node.as_DivideExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::ModuloExpressionNode: {
    const auto &n = node.as_ModuloExpressionNode();
    print_node_field(out, "left", n.left);
    print_node_field(out, "right", n.right);
    break;
  }
  case NodeType::RefExpressionNode: {
    const auto &n = node.as_RefExpressionNode();
    print_field(out, "is_const", n.is_const);
    print_field(out, "is_move", n.is_move);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::AwaitExpressionNode: {
    const auto &n = node.as_AwaitExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::NotExpressionNode: {
    const auto &n = node.as_NotExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::BitwiseNotExpressionNode: {
    const auto &n = node.as_BitwiseNotExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::DerefExpressionNode: {
    const auto &n = node.as_DerefExpressionNode();
    print_field(out, "is_const", n.is_const);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::PositiveExpressionNode: {
    const auto &n = node.as_PositiveExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::NegativeExpressionNode: {
    const auto &n = node.as_NegativeExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::EllipsisExpressionNode: {
    const auto &n = node.as_EllipsisExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::FieldAccessExpressionNode: {
    const auto &n = node.as_FieldAccessExpressionNode();
    print_node_field(out, "object", n.object);
    print_node_field(out, "field", n.field);
    break;
  }
  case NodeType::NumericFieldAccessExpressionNode: {
    const auto &n = node.as_NumericFieldAccessExpressionNode();
    print_node_field(out, "object", n.object);
    print_token_field(out, "lit", n.lit);
    break;
  }
  case NodeType::IndexingExpressionNode: {
    const auto &n = node.as_IndexingExpressionNode();
    print_node_field(out, "object", n.object);
    print_node_field(out, "index", n.index);
    break;
  }
  case NodeType::FunctionArgumentNode: {
    const auto &n = node.as_FunctionArgumentNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::FunctionCallExpressionNode: {
    const auto &n = node.as_FunctionCallExpressionNode();
    print_node_field(out, "callee", n.callee);
    print_node_field(out, "args", n.args);
    break;
  }
  case NodeType::ScopeResolutionExpressionNode: {
    const auto &n = node.as_ScopeResolutionExpressionNode();
    print_node_field(out, "scope", n.scope);
    print_node_field(out, "name", n.name);
    break;
  }
  case NodeType::PreIncrementStatementNode: {
    const auto &n = node.as_PreIncrementStatementNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::PostIncrementStatementNode: {
    const auto &n = node.as_PostIncrementStatementNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::PreDecrementStatementNode: {
    const auto &n = node.as_PreDecrementStatementNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::PostDecrementStatementNode: {
    const auto &n = node.as_PostDecrementStatementNode();
    print_node_field(out, "target", n.target);
    break;
  }
  case NodeType::BlockStatementNode: {
    const auto &n = node.as_BlockStatementNode();
    print_node_field(out, "stmts", n.stmts);
    break;
  }
  case NodeType::ThrowStatementNode: {
    const auto &n = node.as_ThrowStatementNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::OperatorIdentAddNode: {
    break;
  }
  case NodeType::OperatorIdentSubNode: {
    break;
  }
  case NodeType::OperatorIdentStarNode: {
    break;
  }
  case NodeType::OperatorIdentDivNode: {
    break;
  }
  case NodeType::OperatorIdentModNode: {
    break;
  }
  case NodeType::OperatorIdentIncNode: {
    break;
  }
  case NodeType::OperatorIdentDecNode: {
    break;
  }
  case NodeType::OperatorIdentEqNode: {
    break;
  }
  case NodeType::OperatorIdentNeqNode: {
    break;
  }
  case NodeType::OperatorIdentGtNode: {
    break;
  }
  case NodeType::OperatorIdentLtNode: {
    break;
  }
  case NodeType::OperatorIdentGteNode: {
    break;
  }
  case NodeType::OperatorIdentLteNode: {
    break;
  }
  case NodeType::OperatorIdentNotNode: {
    break;
  }
  case NodeType::OperatorIdentAndNode: {
    break;
  }
  case NodeType::OperatorIdentOrNode: {
    break;
  }
  case NodeType::OperatorIdentBitwiseNotNode: {
    break;
  }
  case NodeType::OperatorIdentAmpersandNode: {
    break;
  }
  case NodeType::OperatorIdentBitwiseOrNode: {
    break;
  }
  case NodeType::OperatorIdentBitwiseXorNode: {
    break;
  }
  case NodeType::OperatorIdentLeftShiftNode: {
    break;
  }
  case NodeType::OperatorIdentRightShiftNode: {
    break;
  }
  case NodeType::OperatorIdentAssignNode: {
    break;
  }
  case NodeType::OperatorIdentAddAssignNode: {
    break;
  }
  case NodeType::OperatorIdentSubAssignNode: {
    break;
  }
  case NodeType::OperatorIdentMulAssignNode: {
    break;
  }
  case NodeType::OperatorIdentDivAssignNode: {
    break;
  }
  case NodeType::OperatorIdentModAssignNode: {
    break;
  }
  case NodeType::OperatorIdentBitwiseAndAssignNode: {
    break;
  }
  case NodeType::OperatorIdentBitwiseOrAssignNode: {
    break;
  }
  case NodeType::OperatorIdentBitwiseXorAssignNode: {
    break;
  }
  case NodeType::OperatorIdentLeftShiftAssignNode: {
    break;
  }
  case NodeType::OperatorIdentRightShiftAssignNode: {
    break;
  }
  case NodeType::OperatorIdentIxNode: {
    break;
  }
  case NodeType::OperatorIdentFuncallNode: {
    break;
  }
  case NodeType::OperatorIdentIxAssignNode: {
    break;
  }
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
  case NodeType::OperatorFunctionDeclarationNode: {
    const auto &n = node.as_OperatorFunctionDeclarationNode();
    print_node_field(out, "operator_ident", n.operator_ident);
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::AssignmentStatementNode: {
    const auto &n = node.as_AssignmentStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::AddAssignStatementNode: {
    const auto &n = node.as_AddAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::SubAssignStatementNode: {
    const auto &n = node.as_SubAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::MulAssignStatementNode: {
    const auto &n = node.as_MulAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::DivAssignStatementNode: {
    const auto &n = node.as_DivAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::ModAssignStatementNode: {
    const auto &n = node.as_ModAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::LeftShiftAssignStatementNode: {
    const auto &n = node.as_LeftShiftAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::RightShiftAssignStatementNode: {
    const auto &n = node.as_RightShiftAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::BitwiseAndAssignStatementNode: {
    const auto &n = node.as_BitwiseAndAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::BitwiseOrAssignStatementNode: {
    const auto &n = node.as_BitwiseOrAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::BitwiseXorAssignStatementNode: {
    const auto &n = node.as_BitwiseXorAssignStatementNode();
    print_node_field(out, "target", n.target);
    print_node_field(out, "expression", n.expression);
    break;
  }
  case NodeType::ForInStatementNode: {
    const auto &n = node.as_ForInStatementNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "vars", n.vars);
    print_node_field(out, "iterable", n.iterable);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::WhileStatementNode: {
    const auto &n = node.as_WhileStatementNode();
    print_node_field(out, "introductory_decls", n.introductory_decls);
    print_node_field(out, "condition", n.condition);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::LabelStatementNode: {
    const auto &n = node.as_LabelStatementNode();
    print_node_field(out, "label", n.label);
    break;
  }
  case NodeType::GotoStatementNode: {
    const auto &n = node.as_GotoStatementNode();
    print_node_field(out, "label", n.label);
    break;
  }
  case NodeType::ContinueStatementNode: {
    break;
  }
  case NodeType::ReturnStatementNode: {
    const auto &n = node.as_ReturnStatementNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::FunctionParameterNode: {
    const auto &n = node.as_FunctionParameterNode();
    print_field(out, "variadic", n.variadic);
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
    print_node_field(out, "expression", n.expression);
    print_node_field(out, "stmts", n.stmts);
    print_field(out, "is_default", n.is_default);
    print_field(out, "is_deleted", n.is_deleted);
    break;
  }
  case NodeType::ImplicitParameterListNode: {
    const auto &n = node.as_ImplicitParameterListNode();
    print_node_field(out, "parameters", n.parameters);
    break;
  }
  case NodeType::FunctionSignatureCaptureAnnotationListNode: {
    const auto &n = node.as_FunctionSignatureCaptureAnnotationListNode();
    print_node_field(out, "captures", n.captures);
    break;
  }
  case NodeType::FunctionSignatureCaptureAnnotationNode: {
    const auto &n = node.as_FunctionSignatureCaptureAnnotationNode();
    Text kind;
    if (n.kind == FunctionCaptureKind::Copy) {
      kind = "copy";
    } else if (n.kind == FunctionCaptureKind::Move) {
      kind = "move";
    } else if (n.kind == FunctionCaptureKind::Ref) {
      kind = "ref";
    } else {
      throw std::runtime_error("unreachable");
    }
    print_field(out, "kind", kind);
    print_node_field(out, "var", n.var);
    break;
  }
  case NodeType::FunctionDeclarationNode: {
    const auto &n = node.as_FunctionDeclarationNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::FunctionExpressionNode: {
    const auto &n = node.as_FunctionExpressionNode();
    print_node_field(out, "signature", n.signature);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::LambdaExpressionNode: {
    const auto &n = node.as_LambdaExpressionNode();
    print_node_field(out, "parameters", n.parameters);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::TypeDeclarationNode: {
    const auto &n = node.as_TypeDeclarationNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "type_expr", n.type_expr);
    break;
  }
  case NodeType::ClassStaticDeclarationNode: {
    const auto &n = node.as_ClassStaticDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ClassConstDeclarationNode: {
    const auto &n = node.as_ClassConstDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ClassCopyDeclarationNode: {
    const auto &n = node.as_ClassCopyDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ClassMoveDeclarationNode: {
    const auto &n = node.as_ClassMoveDeclarationNode();
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
  case NodeType::ClassDeclarationNode: {
    const auto &n = node.as_ClassDeclarationNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "base_class_list", n.base_class_list);
    print_node_field(out, "implicit_parameter_list", n.implicit_parameter_list);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::ConceptDeclarationNode: {
    const auto &n = node.as_ConceptDeclarationNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "generic_parameter_list", n.generic_parameter_list);
    print_node_field(out, "base_concept_list", n.base_concept_list);
    print_node_field(out, "implicit_parameter_list", n.implicit_parameter_list);
    print_node_field(out, "body", n.body);
    break;
  }
  case NodeType::ClassBodyNode: {
    const auto &n = node.as_ClassBodyNode();
    print_node_field(out, "decls", n.decls);
    break;
  }
  case NodeType::GenericParameterNode: {
    const auto &n = node.as_GenericParameterNode();
    print_field(out, "is_const", n.is_const);
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
  case NodeType::ImplTypeExpressionNode: {
    const auto &n = node.as_ImplTypeExpressionNode();
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::AnyTypeExpressionNode: {
    const auto &n = node.as_AnyTypeExpressionNode();
    print_node_field(out, "type", n.type);
    break;
  }
  case NodeType::AsyncExpressionNode: {
    const auto &n = node.as_AsyncExpressionNode();
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
    print_field(out, "value", n.value);
    break;
  }
  case NodeType::ThisLiteralNode: {
    break;
  }
  case NodeType::SuperLiteralNode: {
    break;
  }
  case NodeType::SelfTypeNode: {
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
    }
    print_field(out, "visibility", visibility);
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::CopyExpressionNode: {
    const auto &n = node.as_CopyExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::MoveExpressionNode: {
    const auto &n = node.as_MoveExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::CopyCtorNameNode: {
    break;
  }
  case NodeType::MoveCtorNameNode: {
    break;
  }
  case NodeType::ImplicitDeclarationNode: {
    const auto &n = node.as_ImplicitDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::OpenDeclarationNode: {
    const auto &n = node.as_OpenDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::OverrideDeclarationNode: {
    const auto &n = node.as_OverrideDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::DefaultDeclarationNode: {
    const auto &n = node.as_DefaultDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::DefaultLiteralNode: {
    break;
  }
  case NodeType::PrimitiveTypeNode: {
    const auto &n = node.as_PrimitiveTypeNode();
    print_token_field(out, "token", n.token);
    break;
  }
  case NodeType::AutoTypeNode: {
    break;
  }
  case NodeType::ImportDeclarationNode: {
    const auto &n = node.as_ImportDeclarationNode();
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
  case NodeType::ModuleDeclarationNode: {
    const auto &n = node.as_ModuleDeclarationNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "decls", n.decls);
    break;
  }
  case NodeType::AsyncDeclarationNode: {
    const auto &n = node.as_AsyncDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ExternDeclarationNode: {
    const auto &n = node.as_ExternDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::ExportDeclarationNode: {
    const auto &n = node.as_ExportDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::RecordDeclarationNode: {
    const auto &n = node.as_RecordDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::UnionDeclarationNode: {
    const auto &n = node.as_UnionDeclarationNode();
    print_node_field(out, "decl", n.decl);
    break;
  }
  case NodeType::EnumDeclarationNode: {
    const auto &n = node.as_EnumDeclarationNode();
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
  case NodeType::InlineExpressionNode: {
    const auto &n = node.as_InlineExpressionNode();
    print_node_field(out, "expr", n.expr);
    break;
  }
  case NodeType::BreakStatementNode: {
    break;
  }
  case NodeType::AnnotationNode: {
    const auto &n = node.as_AnnotationNode();
    print_node_field(out, "name", n.name);
    print_node_field(out, "args", n.args);
    print_node_field(out, "stmt", n.stmt);
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

void NodeFormatter::print_node_field(
    AbstractString &out, Text name, const List<NodeId> &nodes_value
) {
  if (nodes_value.size() == 0)
    return;
  open_line(out);
  out.append(name);
  out.append("=[");
  int old_fields_printed = m_fields_printed;
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

void NodeFormatter::print_node_field(
    AbstractString &out, Text name, const Option<List<NodeId>> &nodes
) {
  if (!nodes.has_value() || nodes.value().size() == 0)
    return;
  open_line(out);
  out.append(name);
  out.append("=[");
  const auto &nodes_value = nodes.value();
  int old_fields_printed = m_fields_printed;
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

void NodeFormatter::print_node_field(AbstractString &out, Text name, Option<NodeId> node_id) {
  if (!node_id.has_value())
    return;
  open_line(out);
  out.append(name);
  out.append('=');
  format_node(out, node_id.value());
}

void NodeFormatter::print_token_field(AbstractString &out, Text name, TokenId token_id) {
  open_line(out);
  out.append(name);
  out.append('=');
  m_token_formatter.format_token(out, token_id);
}

void NodeFormatter::print_field(AbstractString &out, Text name, Text value) {
  open_line(out);
  out.append(name);
  out.append('=');
  out.append(value);
}

void NodeFormatter::print_field(AbstractString &out, Text name, bool value) {
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
  for (int i = 0; i < m_current_indent; ++i) {
    out.append(' ');
  }
}

} // namespace amelia
