#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.h"
#include "data/lexer/token_formatter.h"
#include "data/parser/abstract_node_repository.h"
#include "data/parser/node.h"
#include "prelude.h"

namespace amelia {

class NodeFormatter {
public:
  NodeFormatter(const AbstractNodeRepository &node_repo, const AbstractTokenRepository &token_repo)
      : m_token_formatter(TokenFormatter(token_repo)), m_node_repo(node_repo) {}

  void format_node(AbstractString &out, size_t node_id) const {
    format_node_with_indent(out, node_id, 0);
  }

private:
  void format_node_with_indent(AbstractString &out, size_t node_id, int indent) const {
    const Node &node = m_node_repo.get_node(node_id);
    node_type_to_string(out, node.type());
    out.append('(');
    size_t prior_size = out.text().size();
    switch (node.type()) {
    case NodeType::IdentifierNode: {
      const auto &n = node.as_IdentifierNode();
      print_token_field(out, "name", n.name, indent + 2);
      break;
    }
    case NodeType::EmptyStatementNode: {
      break;
    }
    case NodeType::LetStatementNode: {
      const auto &n = node.as_LetStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::ConstStatementNode: {
      const auto &n = node.as_ConstStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::StringLiteralNode: {
      const auto &n = node.as_StringLiteralNode();
      print_token_field(out, "value", n.value, indent + 2);
      break;
    }
    case NodeType::NumberLiteralNode: {
      const auto &n = node.as_NumberLiteralNode();
      print_token_field(out, "value", n.value, indent + 2);
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
      print_node_field_with_comma(out, "key", n.key, indent + 2);
      print_node_field(out, "value", n.value, indent + 2);
      break;
    }
    case NodeType::ExpressionStatementNode: {
      const auto &n = node.as_ExpressionStatementNode();
      print_node_field(out, "expr", n.expr, indent + 2);
      break;
    }
    case NodeType::IfThenStatementNode: {
      const auto &n = node.as_IfThenStatementNode();
      print_node_field_with_comma(out, "condition", n.condition, indent + 2);
      print_node_field(out, "then_branch", n.then_branch, indent + 2);
      break;
    }
    case NodeType::IfThenElseStatementNode: {
      const auto &n = node.as_IfThenElseStatementNode();
      print_node_field_with_comma(out, "condition", n.condition, indent + 2);
      print_node_field_with_comma(out, "then_branch", n.then_branch, indent + 2);
      print_node_field(out, "else_branch", n.else_branch, indent + 2);
      break;
    }
    case NodeType::IfThenElseExpressionNode: {
      const auto &n = node.as_IfThenElseExpressionNode();
      print_node_field_with_comma(out, "condition", n.condition, indent + 2);
      print_node_field_with_comma(out, "then_branch", n.then_branch, indent + 2);
      print_node_field(out, "else_branch", n.else_branch, indent + 2);
      break;
    }
    case NodeType::CatchClauseNode: {
      const auto &n = node.as_CatchClauseNode();
      print_node_field_with_comma(out, "exc_type", n.exc_type, indent + 2);
      print_node_field(out, "body", n.body, indent + 2);
      break;
    }
    case NodeType::CatchClauseBindingNode: {
      const auto &n = node.as_CatchClauseBindingNode();
      print_token_field_with_comma(out, "var", n.var, indent + 2);
      print_node_field_with_comma(out, "exc_type", n.exc_type, indent + 2);
      print_node_field(out, "body", n.body, indent + 2);
      break;
    }
    case NodeType::TryCatchExpressionNode: {
      const auto &n = node.as_TryCatchExpressionNode();
      print_node_field_with_comma(out, "try_block", n.try_block, indent + 2);
      print_node_list_field(out, "clauses", n.clauses.data(), indent + 2);
      break;
    }
    case NodeType::CaseClauseNode: {
      const auto &n = node.as_CaseClauseNode();
      print_node_field_with_comma(out, "expr", n.expr, indent + 2);
      print_node_field(out, "body", n.body, indent + 2);
      break;
    }
    case NodeType::SwitchExpressionNode: {
      const auto &n = node.as_SwitchExpressionNode();
      print_node_field_with_comma(out, "expr", n.expr, indent + 2);
      print_node_list_field(out, "clauses", n.clauses.data(), indent + 2);
      break;
    }
    case NodeType::OrExpressionNode: {
      const auto &n = node.as_OrExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::AndExpressionNode: {
      const auto &n = node.as_AndExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::BitwiseOrExpressionNode: {
      const auto &n = node.as_BitwiseOrExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::BitwiseXorExpressionNode: {
      const auto &n = node.as_BitwiseXorExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::BitwiseAndExpressionNode: {
      const auto &n = node.as_BitwiseAndExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::EqualsExpressionNode: {
      const auto &n = node.as_EqualsExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::NotEqualsExpressionNode: {
      const auto &n = node.as_NotEqualsExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::GreaterEqualsExpressionNode: {
      const auto &n = node.as_GreaterEqualsExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::LessEqualsExpressionNode: {
      const auto &n = node.as_LessEqualsExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::GreaterExpressionNode: {
      const auto &n = node.as_GreaterExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::LessExpressionNode: {
      const auto &n = node.as_LessExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::LeftShiftExpressionNode: {
      const auto &n = node.as_LeftShiftExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::RightShiftExpressionNode: {
      const auto &n = node.as_RightShiftExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::AddExpressionNode: {
      const auto &n = node.as_AddExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::SubtractExpressionNode: {
      const auto &n = node.as_SubtractExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::MultiplyExpressionNode: {
      const auto &n = node.as_MultiplyExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::DivideExpressionNode: {
      const auto &n = node.as_DivideExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::ModuloExpressionNode: {
      const auto &n = node.as_ModuloExpressionNode();
      print_node_field_with_comma(out, "left", n.left, indent + 2);
      print_node_field(out, "right", n.right, indent + 2);
      break;
    }
    case NodeType::RefExpressionNode: {
      const auto &n = node.as_RefExpressionNode();
      print_node_field(out, "expr", n.expr, indent + 2);
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
      print_node_field(out, "expr", n.expr, indent + 2);
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
      print_node_field_with_comma(out, "object", n.object, indent + 2);
      print_node_field(out, "field", n.field, indent + 2);
      break;
    }
    case NodeType::NumericFieldAccessExpressionNode: {
      const auto &n = node.as_NumericFieldAccessExpressionNode();
      print_node_field_with_comma(out, "object", n.object, indent + 2);
      print_token_field(out, "field", n.field, indent + 2);
      break;
    }
    case NodeType::IndexingExpressionNode: {
      const auto &n = node.as_IndexingExpressionNode();
      print_node_field_with_comma(out, "object", n.object, indent + 2);
      print_node_field(out, "index", n.index, indent + 2);
      break;
    }
    case NodeType::PositionalFunctionArgumentNode: {
      const auto &n = node.as_PositionalFunctionArgumentNode();
      print_node_field(out, "expr", n.expr, indent + 2);
      break;
    }
    case NodeType::NamedFunctionArgumentNode: {
      const auto &n = node.as_NamedFunctionArgumentNode();
      print_node_field_with_comma(out, "name", n.name, indent + 2);
      print_node_field(out, "expr", n.expr, indent + 2);
      break;
    }
    case NodeType::FunctionCallExpressionNode: {
      const auto &n = node.as_FunctionCallExpressionNode();
      print_node_field_with_comma(out, "callee", n.callee, indent + 2);
      print_node_list_field(out, "args", n.args.data(), indent + 2);
      break;
    }
    case NodeType::ScopeResolutionExpressionNode: {
      const auto &n = node.as_ScopeResolutionExpressionNode();
      print_node_field_with_comma(out, "scope", n.scope, indent + 2);
      print_node_field(out, "name", n.name, indent + 2);
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
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::AddAssignStatementNode: {
      const auto &n = node.as_AddAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::SubAssignStatementNode: {
      const auto &n = node.as_SubAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::MulAssignStatementNode: {
      const auto &n = node.as_MulAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::DivAssignStatementNode: {
      const auto &n = node.as_DivAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::ModAssignStatementNode: {
      const auto &n = node.as_ModAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::LeftShiftAssignStatementNode: {
      const auto &n = node.as_LeftShiftAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::RightShiftAssignStatementNode: {
      const auto &n = node.as_RightShiftAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::BitwiseAndAssignStatementNode: {
      const auto &n = node.as_BitwiseAndAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::BitwiseOrAssignStatementNode: {
      const auto &n = node.as_BitwiseOrAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::BitwiseXorAssignStatementNode: {
      const auto &n = node.as_BitwiseXorAssignStatementNode();
      print_node_field_with_comma(out, "target", n.target, indent + 2);
      print_node_field(out, "expression", n.expression, indent + 2);
      break;
    }
    case NodeType::ForInStatementNode: {
      const auto &n = node.as_ForInStatementNode();
      print_node_field_with_comma(out, "var", n.var, indent + 2);
      print_node_field_with_comma(out, "iterable", n.iterable, indent + 2);
      print_node_field(out, "body", n.body, indent + 2);
      break;
    }
    case NodeType::WhileStatementNode: {
      const auto &n = node.as_WhileStatementNode();
      print_node_field_with_comma(out, "condition", n.condition, indent + 2);
      print_node_field(out, "body", n.body, indent + 2);
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

  void print_node_list_field(AbstractString &out, Text name, ConstSlice<NodeId> nodes, int indent)
      const {
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

  void print_node_field(AbstractString &out, Text name, NodeId node_id, int indent) const {
    open_line(out, indent);
    out.append(name);
    out.append('=');
    format_node_with_indent(out, node_id, indent);
  }

  void print_node_field_with_comma(AbstractString &out, Text name, NodeId node_id, int indent)
      const {
    print_node_field(out, name, node_id, indent);
    out.append(',');
  }

  void print_token_field(AbstractString &out, Text name, NodeId token_id, int indent) const {
    open_line(out, indent);
    out.append(name);
    out.append('=');
    m_token_formatter.format_token(out, token_id);
  }

  void print_token_field_with_comma(AbstractString &out, Text name, NodeId token_id, int indent)
      const {
    print_token_field(out, name, token_id, indent);
    out.append(',');
  }

  static void open_line(AbstractString &out, int indent) {
    out.append('\n');
    print_indent(out, indent);
  }

  static void open_line_comma(AbstractString &out, int indent) {
    out.append(",\n");
    print_indent(out, indent);
  }

  static void print_indent(AbstractString &out, int indent) {
    for (int i = 0; i < indent; ++i) {
      out.append(' ');
    }
  }

  const TokenFormatter m_token_formatter;
  const AbstractNodeRepository &m_node_repo;
};

} // namespace amelia
