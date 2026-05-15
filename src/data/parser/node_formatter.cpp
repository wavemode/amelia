#include "node_formatter.h"

namespace amelia {
void NodeFormatter::format_node(AbstractString &out, NodeId node_id) const {
  format_node_with_indent(out, node_id, 0);
}

void NodeFormatter::format_node_with_indent(AbstractString &out, NodeId node_id, int indent) const {
  const Node &node = m_node_repo.get_node(node_id);
  node_type_to_string(out, node.type());
  out.append('(');
  size_t prior_size = out.text().size();
  switch (node.type()) {
  case NodeType::IdentifierNode: {
    const auto &n = node.as_IdentifierNode();
    print_token_field(out, "token", n.token, indent + 2);
    break;
  }
  case NodeType::EmptyStatementNode: {
    break;
  }
  case NodeType::LetStatementNode: {
    const auto &n = node.as_LetStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    break;
  }
  case NodeType::LetAssignmentStatementNode: {
    const auto &n = node.as_LetAssignmentStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::LetAssignAnnotationNode: {
    const auto &n = node.as_LetAssignAnnotationNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "type", n.type, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::LetAnnotationNode: {
    const auto &n = node.as_LetAnnotationNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "type", n.type, indent + 2);
    break;
  }
  case NodeType::ConstStatementNode: {
    const auto &n = node.as_ConstStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    break;
  }
  case NodeType::ConstAssignmentStatementNode: {
    const auto &n = node.as_ConstAssignmentStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::ConstAssignAnnotationNode: {
    const auto &n = node.as_ConstAssignAnnotationNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "type", n.type, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::ConstAnnotationNode: {
    const auto &n = node.as_ConstAnnotationNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "type", n.type, indent + 2);
    break;
  }
  case NodeType::StringLiteralNode: {
    const auto &n = node.as_StringLiteralNode();
    print_token_field(out, "lit", n.lit, indent + 2);
    break;
  }
  case NodeType::NumberLiteralNode: {
    const auto &n = node.as_NumberLiteralNode();
    print_token_field(out, "lit", n.lit, indent + 2);
    break;
  }
  case NodeType::ParenthesizedExpressionNode: {
    const auto &n = node.as_ParenthesizedExpressionNode();
    print_node_list_field(out, "exprs", n.exprs.data(), indent + 2);
    break;
  }
  case NodeType::ModuleNode: {
    const auto &n = node.as_ModuleNode();
    print_node_list_field(out, "decls", n.decls.data(), indent + 2);
    break;
  }
  case NodeType::ArrayLiteralNode: {
    const auto &n = node.as_ArrayLiteralNode();
    print_node_list_field(out, "exprs", n.exprs.data(), indent + 2);
    break;
  }
  case NodeType::BlockExpressionNode: {
    const auto &n = node.as_BlockExpressionNode();
    print_node_list_field(out, "stmts", n.stmts.data(), indent + 2);
    break;
  }
  case NodeType::ObjectLiteralNode: {
    const auto &n = node.as_ObjectLiteralNode();
    print_node_list_field(out, "entries", n.entries.data(), indent + 2);
    break;
  }
  case NodeType::KeyValueEntryNode: {
    const auto &n = node.as_KeyValueEntryNode();
    print_node_field(out, "key", n.key, indent + 2);
    print_node_field_with_comma(out, "value", n.value, indent + 2);
    break;
  }
  case NodeType::ExpressionStatementNode: {
    const auto &n = node.as_ExpressionStatementNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::IfThenStatementNode: {
    const auto &n = node.as_IfThenStatementNode();
    print_node_field(out, "condition", n.condition, indent + 2);
    print_node_field_with_comma(out, "then_branch", n.then_branch, indent + 2);
    break;
  }
  case NodeType::IfThenElseStatementNode: {
    const auto &n = node.as_IfThenElseStatementNode();
    print_node_field(out, "condition", n.condition, indent + 2);
    print_node_field_with_comma(out, "then_branch", n.then_branch, indent + 2);
    print_node_field_with_comma(out, "else_branch", n.else_branch, indent + 2);
    break;
  }
  case NodeType::IfThenElseExpressionNode: {
    const auto &n = node.as_IfThenElseExpressionNode();
    print_node_field(out, "condition", n.condition, indent + 2);
    print_node_field_with_comma(out, "then_branch", n.then_branch, indent + 2);
    print_node_field_with_comma(out, "else_branch", n.else_branch, indent + 2);
    break;
  }
  case NodeType::CatchClauseNode: {
    const auto &n = node.as_CatchClauseNode();
    print_node_field(out, "exc_type", n.exc_type, indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::CatchClauseBindingNode: {
    const auto &n = node.as_CatchClauseBindingNode();
    print_node_field(out, "var", n.var, indent + 2);
    print_node_field_with_comma(out, "exc_type", n.exc_type, indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::TryCatchExpressionNode: {
    const auto &n = node.as_TryCatchExpressionNode();
    print_node_field(out, "try_block", n.try_block, indent + 2);
    print_node_list_field_with_comma(out, "clauses", n.clauses.data(), indent + 2);
    break;
  }
  case NodeType::CaseClauseNode: {
    const auto &n = node.as_CaseClauseNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::SwitchExpressionNode: {
    const auto &n = node.as_SwitchExpressionNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    print_node_list_field_with_comma(out, "clauses", n.clauses.data(), indent + 2);
    break;
  }
  case NodeType::OrExpressionNode: {
    const auto &n = node.as_OrExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::AndExpressionNode: {
    const auto &n = node.as_AndExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::BitwiseOrExpressionNode: {
    const auto &n = node.as_BitwiseOrExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::BitwiseXorExpressionNode: {
    const auto &n = node.as_BitwiseXorExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::BitwiseAndExpressionNode: {
    const auto &n = node.as_BitwiseAndExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::EqualsExpressionNode: {
    const auto &n = node.as_EqualsExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::NotEqualsExpressionNode: {
    const auto &n = node.as_NotEqualsExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::GreaterEqualsExpressionNode: {
    const auto &n = node.as_GreaterEqualsExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::LessEqualsExpressionNode: {
    const auto &n = node.as_LessEqualsExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::GreaterExpressionNode: {
    const auto &n = node.as_GreaterExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::LessExpressionNode: {
    const auto &n = node.as_LessExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::LeftShiftExpressionNode: {
    const auto &n = node.as_LeftShiftExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::RightShiftExpressionNode: {
    const auto &n = node.as_RightShiftExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::AddExpressionNode: {
    const auto &n = node.as_AddExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::SubtractExpressionNode: {
    const auto &n = node.as_SubtractExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::MultiplyExpressionNode: {
    const auto &n = node.as_MultiplyExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::DivideExpressionNode: {
    const auto &n = node.as_DivideExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::ModuloExpressionNode: {
    const auto &n = node.as_ModuloExpressionNode();
    print_node_field(out, "left", n.left, indent + 2);
    print_node_field_with_comma(out, "right", n.right, indent + 2);
    break;
  }
  case NodeType::RefExpressionNode: {
    const auto &n = node.as_RefExpressionNode();
    print_field(out, "is_const", n.is_const, indent + 2);
    print_node_field_with_comma(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::AwaitExpressionNode: {
    const auto &n = node.as_AwaitExpressionNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::NotExpressionNode: {
    const auto &n = node.as_NotExpressionNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::BitwiseNotExpressionNode: {
    const auto &n = node.as_BitwiseNotExpressionNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::DerefExpressionNode: {
    const auto &n = node.as_DerefExpressionNode();
    print_field(out, "is_const", n.is_const, indent + 2);
    print_node_field_with_comma(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::PositiveExpressionNode: {
    const auto &n = node.as_PositiveExpressionNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::NegativeExpressionNode: {
    const auto &n = node.as_NegativeExpressionNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::EllipsisExpressionNode: {
    const auto &n = node.as_EllipsisExpressionNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::FieldAccessExpressionNode: {
    const auto &n = node.as_FieldAccessExpressionNode();
    print_node_field(out, "object", n.object, indent + 2);
    print_node_field_with_comma(out, "field", n.field, indent + 2);
    break;
  }
  case NodeType::NumericFieldAccessExpressionNode: {
    const auto &n = node.as_NumericFieldAccessExpressionNode();
    print_node_field(out, "object", n.object, indent + 2);
    print_token_field_with_comma(out, "lit", n.lit, indent + 2);
    break;
  }
  case NodeType::IndexingExpressionNode: {
    const auto &n = node.as_IndexingExpressionNode();
    print_node_field(out, "object", n.object, indent + 2);
    print_node_field_with_comma(out, "index", n.index, indent + 2);
    break;
  }
  case NodeType::PositionalFunctionArgumentNode: {
    const auto &n = node.as_PositionalFunctionArgumentNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::NamedFunctionArgumentNode: {
    const auto &n = node.as_NamedFunctionArgumentNode();
    print_node_field(out, "name", n.name, indent + 2);
    print_node_field_with_comma(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::FunctionCallExpressionNode: {
    const auto &n = node.as_FunctionCallExpressionNode();
    print_node_field(out, "callee", n.callee, indent + 2);
    print_node_list_field_with_comma(out, "args", n.args.data(), indent + 2);
    break;
  }
  case NodeType::ScopeResolutionExpressionNode: {
    const auto &n = node.as_ScopeResolutionExpressionNode();
    print_node_field(out, "scope", n.scope, indent + 2);
    print_node_field_with_comma(out, "name", n.name, indent + 2);
    break;
  }
  case NodeType::PreIncrementStatementNode: {
    const auto &n = node.as_PreIncrementStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    break;
  }
  case NodeType::PostIncrementStatementNode: {
    const auto &n = node.as_PostIncrementStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    break;
  }
  case NodeType::PreDecrementStatementNode: {
    const auto &n = node.as_PreDecrementStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    break;
  }
  case NodeType::PostDecrementStatementNode: {
    const auto &n = node.as_PostDecrementStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    break;
  }
  case NodeType::BlockStatementNode: {
    const auto &n = node.as_BlockStatementNode();
    print_node_list_field(out, "stmts", n.stmts.data(), indent + 2);
    break;
  }
  case NodeType::ThrowStatementNode: {
    const auto &n = node.as_ThrowStatementNode();
    print_node_field(out, "expr", n.expr, indent + 2);
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
  case NodeType::OperatorIdentBitwiseAndNode: {
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
  case NodeType::OperatorIdentAsNode: {
    const auto &n = node.as_OperatorIdentAsNode();
    print_node_field(out, "type", n.type, indent + 2);
    break;
  }
  case NodeType::OperatorIdentifierNode: {
    const auto &n = node.as_OperatorIdentifierNode();
    print_node_field(out, "operator_node", n.operator_node, indent + 2);
    break;
  }
  case NodeType::AssignmentStatementNode: {
    const auto &n = node.as_AssignmentStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::AddAssignStatementNode: {
    const auto &n = node.as_AddAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::SubAssignStatementNode: {
    const auto &n = node.as_SubAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::MulAssignStatementNode: {
    const auto &n = node.as_MulAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::DivAssignStatementNode: {
    const auto &n = node.as_DivAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::ModAssignStatementNode: {
    const auto &n = node.as_ModAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::LeftShiftAssignStatementNode: {
    const auto &n = node.as_LeftShiftAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::RightShiftAssignStatementNode: {
    const auto &n = node.as_RightShiftAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::BitwiseAndAssignStatementNode: {
    const auto &n = node.as_BitwiseAndAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::BitwiseOrAssignStatementNode: {
    const auto &n = node.as_BitwiseOrAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::BitwiseXorAssignStatementNode: {
    const auto &n = node.as_BitwiseXorAssignStatementNode();
    print_node_field(out, "target", n.target, indent + 2);
    print_node_field_with_comma(out, "expression", n.expression, indent + 2);
    break;
  }
  case NodeType::ForInStatementNode: {
    const auto &n = node.as_ForInStatementNode();
    print_node_list_field(out, "vars", n.vars.data(), indent + 2);
    print_node_field_with_comma(out, "iterable", n.iterable, indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::WhileStatementNode: {
    const auto &n = node.as_WhileStatementNode();
    print_node_field(out, "condition", n.condition, indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::LabelStatementNode: {
    const auto &n = node.as_LabelStatementNode();
    print_node_field(out, "label", n.label, indent + 2);
    break;
  }
  case NodeType::GotoStatementNode: {
    const auto &n = node.as_GotoStatementNode();
    print_node_field(out, "label", n.label, indent + 2);
    break;
  }
  case NodeType::ContinueStatementNode: {
    break;
  }
  case NodeType::ReturnStatementNode: {
    break;
  }
  case NodeType::ReturnValueStatementNode: {
    const auto &n = node.as_ReturnValueStatementNode();
    print_node_field(out, "expr", n.expr, indent + 2);
    break;
  }
  case NodeType::FunctionParameterNode: {
    const auto &n = node.as_FunctionParameterNode();
    print_field(out, "variadic", n.variadic, indent + 2);
    print_node_field_with_comma(out, "name", n.name, indent + 2);
    print_node_field_with_comma(out, "type", n.type, indent + 2);
    print_node_field_with_comma(out, "default_value", n.default_value, indent + 2);
    break;
  }
  case NodeType::FunctionSignatureNode: {
    const auto &n = node.as_FunctionSignatureNode();
    print_node_list_field(out, "parameters", n.parameters.data(), indent + 2);
    print_node_field_with_comma(
        out, "implicit_parameter_list", n.implicit_parameter_list, indent + 2
    );
    print_node_field_with_comma(out, "capture_list", n.capture_list, indent + 2);
    print_node_field_with_comma(out, "return_type", n.return_type, indent + 2);
    break;
  }
  case NodeType::FunctionBodyNode: {
    const auto &n = node.as_FunctionBodyNode();
    print_node_list_field(out, "stmts", n.stmts.data(), indent + 2);
    break;
  }
  case NodeType::FunctionImplicitParameterListNode: {
    const auto &n = node.as_FunctionImplicitParameterListNode();
    print_node_list_field(out, "parameters", n.parameters.data(), indent + 2);
    break;
  }
  case NodeType::FunctionSignatureCaptureAnnotationListNode: {
    const auto &n = node.as_FunctionSignatureCaptureAnnotationListNode();
    print_node_list_field(out, "captures", n.captures.data(), indent + 2);
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
      throw RuntimeError("unreachable");
    }
    print_field(out, "kind", kind, indent + 2);
    print_node_field_with_comma(out, "var", n.var, indent + 2);
    break;
  }
  case NodeType::FunctionDeclarationStatementNode: {
    const auto &n = node.as_FunctionDeclarationStatementNode();
    print_node_field(out, "name", n.name, indent + 2);
    print_node_field_with_comma(out, "signature", n.signature, indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::FunctionExpressionNode: {
    const auto &n = node.as_FunctionExpressionNode();
    print_node_field(out, "signature", n.signature, indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::LambdaExpressionNode: {
    const auto &n = node.as_LambdaExpressionNode();
    print_node_list_field(out, "parameters", n.parameters.data(), indent + 2);
    print_node_field_with_comma(out, "body", n.body, indent + 2);
    break;
  }
  case NodeType::SliceExpressionNode: {
    const auto &n = node.as_SliceExpressionNode();
    print_field(out, "is_const", n.is_const, indent + 2);
    print_node_field_with_comma(out, "type", n.type, indent + 2);
    break;
  }
  case NodeType::ArrayExpressionNode: {
    const auto &n = node.as_ArrayExpressionNode();
    print_node_field(out, "type", n.type, indent + 2);
    print_node_field_with_comma(out, "size", n.size, indent + 2);
    if (n.elements.has_value()) {
      print_node_list_field_with_comma(out, "elements", n.elements.value().data(), indent + 2);
    }
    break;
  }
  default:
    throw RuntimeError("Unknown node type");
  }
  if (out.text().size() > prior_size) {
    open_line(out, indent);
  }
  out.append(')');
}

void NodeFormatter::print_node_list_field(
    AbstractString &out, Text name, ConstSlice<NodeId> nodes, int indent
) const {
  open_line(out, indent);
  out.append(name);
  out.append("=[");
  if (nodes.size() > 0) {
    for (size_t i = 0; i < nodes.size(); ++i) {
      open_line(out, indent + 2);
      format_node_with_indent(out, nodes[i], indent + 2);
      if (i < nodes.size() - 1) {
        out.append(',');
      }
    }
    open_line(out, indent);
  }
  out.append(']');
}

void NodeFormatter::print_node_field(
    AbstractString &out, Text name, Option<NodeId> node_id, int indent
) const {
  if (!node_id.has_value())
    return;
  open_line(out, indent);
  out.append(name);
  out.append('=');
  format_node_with_indent(out, node_id.value(), indent);
}

void NodeFormatter::print_node_field_with_comma(
    AbstractString &out, Text name, Option<NodeId> node_id, int indent
) const {
  if (!node_id.has_value())
    return;
  out.append(',');
  print_node_field(out, name, node_id, indent);
}

void NodeFormatter::print_token_field(AbstractString &out, Text name, TokenId token_id, int indent)
    const {
  open_line(out, indent);
  out.append(name);
  out.append('=');
  m_token_formatter.format_token(out, token_id);
}

void NodeFormatter::print_token_field_with_comma(
    AbstractString &out, Text name, TokenId token_id, int indent
) const {
  out.append(',');
  print_token_field(out, name, token_id, indent);
}

void NodeFormatter::print_field(AbstractString &out, Text name, Text value, int indent) const {
  open_line(out, indent);
  out.append(name);
  out.append('=');
  out.append(value);
}

void NodeFormatter::print_field(AbstractString &out, Text name, bool value, int indent) const {
  open_line(out, indent);
  out.append(name);
  out.append('=');
  TextUtils::to_string(out, value);
}

void NodeFormatter::print_field_with_comma(AbstractString &out, Text name, Text value, int indent)
    const {
  out.append(',');
  print_field(out, name, value, indent);
}

void NodeFormatter::open_line(AbstractString &out, int indent) {
  out.append('\n');
  print_indent(out, indent);
}

void NodeFormatter::open_line_comma(AbstractString &out, int indent) {
  out.append(",\n");
  print_indent(out, indent);
}

void NodeFormatter::print_indent(AbstractString &out, int indent) {
  for (int i = 0; i < indent; ++i) {
    out.append(' ');
  }
}

void NodeFormatter::print_node_list_field_with_comma(
    AbstractString &out, Text name, ConstSlice<NodeId> nodes, int indent
) const {
  out.append(',');
  print_node_list_field(out, name, nodes, indent);
}

} // namespace amelia
