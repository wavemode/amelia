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
    const Node &generic_node = m_node_repo.get_node(node_id);
    node_type_to_string(out, generic_node.type());
    out.append('(');
    if (generic_node.type() == NodeType::IdentifierNode) {
      const auto &node = generic_node.as_IdentifierNode();
      print_token_field(out, "name", node.name, indent + 2);
    } else if (generic_node.type() == NodeType::LetStatementNode) {
      const auto &node = generic_node.as_LetStatementNode();
      print_node_field_with_comma(out, "target", node.target, indent + 2);
      print_node_field(out, "expression", node.expression, indent + 2);
    } else if (generic_node.type() == NodeType::ConstStatementNode) {
      const auto &node = generic_node.as_ConstStatementNode();
      print_node_field_with_comma(out, "target", node.target, indent + 2);
      print_node_field(out, "expression", node.expression, indent + 2);
    } else if (generic_node.type() == NodeType::StringLiteralNode) {
      const auto &node = generic_node.as_StringLiteralNode();
      print_token_field(out, "value", node.value, indent + 2);
    } else if (generic_node.type() == NodeType::NumberLiteralNode) {
      const auto &node = generic_node.as_NumberLiteralNode();
      print_token_field(out, "value", node.value, indent + 2);
    } else if (generic_node.type() == NodeType::ParenthesizedExpressionNode) {
      const auto &node = generic_node.as_ParenthesizedExpressionNode();
      print_node_list_field(out, "exprs", node.exprs.data(), indent + 2);
    } else if (generic_node.type() == NodeType::ModuleNode) {
      const auto &node = generic_node.as_ModuleNode();
      print_node_list_field(out, "decls", node.decls.data(), indent + 2);
    } else if (generic_node.type() == NodeType::ArrayLiteralNode) {
      const auto &node = generic_node.as_ArrayLiteralNode();
      print_node_list_field(out, "exprs", node.exprs.data(), indent + 2);
    } else if (generic_node.type() == NodeType::BlockExpressionNode) {
      const auto &node = generic_node.as_BlockExpressionNode();
      print_node_list_field(out, "stmts", node.stmts.data(), indent + 2);
    } else if (generic_node.type() == NodeType::ObjectLiteralNode) {
      const auto &node = generic_node.as_ObjectLiteralNode();
      print_node_list_field(out, "entries", node.entries.data(), indent + 2);
    } else if (generic_node.type() == NodeType::KeyValueEntryNode) {
      const auto &node = generic_node.as_KeyValueEntryNode();
      print_node_field_with_comma(out, "key", node.key, indent + 2);
      print_node_field(out, "value", node.value, indent + 2);
    } else if (generic_node.type() == NodeType::ExpressionStatementNode) {
      const auto &node = generic_node.as_ExpressionStatementNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::IfThenExpressionNode) {
      const auto &node = generic_node.as_IfThenExpressionNode();
      print_node_field_with_comma(out, "condition", node.condition, indent + 2);
      print_node_field(out, "then_branch", node.then_branch, indent + 2);
    } else if (generic_node.type() == NodeType::IfThenElseExpressionNode) {
      const auto &node = generic_node.as_IfThenElseExpressionNode();
      print_node_field_with_comma(out, "condition", node.condition, indent + 2);
      print_node_field_with_comma(out, "then_branch", node.then_branch, indent + 2);
      print_node_field(out, "else_branch", node.else_branch, indent + 2);
    } else if (generic_node.type() == NodeType::CatchClauseNode) {
      const auto &node = generic_node.as_CatchClauseNode();
      print_node_field_with_comma(out, "exc_type", node.exc_type, indent + 2);
      print_node_field(out, "body", node.body, indent + 2);
    } else if (generic_node.type() == NodeType::CatchClauseBindingNode) {
      const auto &node = generic_node.as_CatchClauseBindingNode();
      print_token_field_with_comma(out, "var", node.var, indent + 2);
      print_node_field_with_comma(out, "exc_type", node.exc_type, indent + 2);
      print_node_field(out, "body", node.body, indent + 2);
    } else if (generic_node.type() == NodeType::TryCatchExpressionNode) {
      const auto &node = generic_node.as_TryCatchExpressionNode();
      print_node_field_with_comma(out, "try_block", node.try_block, indent + 2);
      print_node_list_field(out, "clauses", node.clauses.data(), indent + 2);
    } else if (generic_node.type() == NodeType::CaseClauseNode) {
      const auto &node = generic_node.as_CaseClauseNode();
      print_node_field_with_comma(out, "expr", node.expr, indent + 2);
      print_node_field(out, "body", node.body, indent + 2);
    } else if (generic_node.type() == NodeType::SwitchExpressionNode) {
      const auto &node = generic_node.as_SwitchExpressionNode();
      print_node_field_with_comma(out, "expr", node.expr, indent + 2);
      print_node_list_field(out, "clauses", node.clauses.data(), indent + 2);
    } else if (generic_node.type() == NodeType::OrExpressionNode) {
      const auto &node = generic_node.as_OrExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::AndExpressionNode) {
      const auto &node = generic_node.as_AndExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::BitwiseOrExpressionNode) {
      const auto &node = generic_node.as_BitwiseOrExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::BitwiseXorExpressionNode) {
      const auto &node = generic_node.as_BitwiseXorExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::BitwiseAndExpressionNode) {
      const auto &node = generic_node.as_BitwiseAndExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::EqualsExpressionNode) {
      const auto &node = generic_node.as_EqualsExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::NotEqualsExpressionNode) {
      const auto &node = generic_node.as_NotEqualsExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::GreaterEqualsExpressionNode) {
      const auto &node = generic_node.as_GreaterEqualsExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::LessEqualsExpressionNode) {
      const auto &node = generic_node.as_LessEqualsExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::GreaterExpressionNode) {
      const auto &node = generic_node.as_GreaterExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::LessExpressionNode) {
      const auto &node = generic_node.as_LessExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::LeftShiftExpressionNode) {
      const auto &node = generic_node.as_LeftShiftExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::RightShiftExpressionNode) {
      const auto &node = generic_node.as_RightShiftExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::AddExpressionNode) {
      const auto &node = generic_node.as_AddExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::SubtractExpressionNode) {
      const auto &node = generic_node.as_SubtractExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::MultiplyExpressionNode) {
      const auto &node = generic_node.as_MultiplyExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::DivideExpressionNode) {
      const auto &node = generic_node.as_DivideExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::ModuloExpressionNode) {
      const auto &node = generic_node.as_ModuloExpressionNode();
      print_node_field_with_comma(out, "left", node.left, indent + 2);
      print_node_field(out, "right", node.right, indent + 2);
    } else if (generic_node.type() == NodeType::RefExpressionNode) {
      const auto &node = generic_node.as_RefExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::AwaitExpressionNode) {
      const auto &node = generic_node.as_AwaitExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::NotExpressionNode) {
      const auto &node = generic_node.as_NotExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::BitwiseNotExpressionNode) {
      const auto &node = generic_node.as_BitwiseNotExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::DerefExpressionNode) {
      const auto &node = generic_node.as_DerefExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::PositiveExpressionNode) {
      const auto &node = generic_node.as_PositiveExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::NegativeExpressionNode) {
      const auto &node = generic_node.as_NegativeExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::EllipsisExpressionNode) {
      const auto &node = generic_node.as_EllipsisExpressionNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::FieldAccessExpressionNode) {
      const auto &node = generic_node.as_FieldAccessExpressionNode();
      print_node_field_with_comma(out, "object", node.object, indent + 2);
      print_node_field(out, "field", node.field, indent + 2);
    } else if (generic_node.type() == NodeType::NumericFieldAccessExpressionNode) {
      const auto &node = generic_node.as_NumericFieldAccessExpressionNode();
      print_node_field_with_comma(out, "object", node.object, indent + 2);
      print_token_field(out, "field", node.field, indent + 2);
    } else if (generic_node.type() == NodeType::IndexingExpressionNode) {
      const auto &node = generic_node.as_IndexingExpressionNode();
      print_node_field_with_comma(out, "object", node.object, indent + 2);
      print_node_field(out, "index", node.index, indent + 2);
    } else if (generic_node.type() == NodeType::PositionalFunctionArgumentNode) {
      const auto &node = generic_node.as_PositionalFunctionArgumentNode();
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::NamedFunctionArgumentNode) {
      const auto &node = generic_node.as_NamedFunctionArgumentNode();
      print_node_field_with_comma(out, "name", node.name, indent + 2);
      print_node_field(out, "expr", node.expr, indent + 2);
    } else if (generic_node.type() == NodeType::FunctionCallExpressionNode) {
      const auto &node = generic_node.as_FunctionCallExpressionNode();
      print_node_field_with_comma(out, "callee", node.callee, indent + 2);
      print_node_list_field(out, "args", node.args.data(), indent + 2);
    } else if (generic_node.type() == NodeType::ScopeResolutionExpressionNode) {
      const auto &node = generic_node.as_ScopeResolutionExpressionNode();
      print_node_field_with_comma(out, "scope", node.scope, indent + 2);
      print_node_field(out, "name", node.name, indent + 2);
    } else if (generic_node.type() == NodeType::PreIncrementStatementNode) {
      const auto &node = generic_node.as_PreIncrementStatementNode();
      print_node_field(out, "target", node.target, indent + 2);
    } else if (generic_node.type() == NodeType::PostIncrementStatementNode) {
      const auto &node = generic_node.as_PostIncrementStatementNode();
      print_node_field(out, "target", node.target, indent + 2);
    } else if (generic_node.type() == NodeType::PreDecrementStatementNode) {
      const auto &node = generic_node.as_PreDecrementStatementNode();
      print_node_field(out, "target", node.target, indent + 2);
    } else if (generic_node.type() == NodeType::PostDecrementStatementNode) {
      const auto &node = generic_node.as_PostDecrementStatementNode();
      print_node_field(out, "target", node.target, indent + 2);
    } else {
      throw RuntimeError("Unknown node type");
    }
    open_line(out, indent);
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
