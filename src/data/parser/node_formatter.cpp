#include "node_formatter.h"
#include "prelude.h"

#include "data/parser/node.h"

namespace amelia {

namespace {

void print_indent(AbstractString &out, int indent) {
  for (int i = 0; i < indent; ++i) {
    out.append(' ');
  }
}

void open_line(AbstractString &out, int indent) {
  out.append('\n');
  print_indent(out, indent);
}

void open_line_comma(AbstractString &out, int indent) {
  out.append(",\n");
  print_indent(out, indent);
}

void print_token_field(
    const TokenFormatter &tf, AbstractString &out, Text name, NodeId token_id, int indent
) {
  open_line(out, indent);
  out.append(name);
  out.append('=');
  tf.format_token(out, token_id);
}

void print_token_field_with_comma(
    const TokenFormatter &tf, AbstractString &out, Text name, NodeId token_id, int indent
) {
  print_token_field(tf, out, name, token_id, indent);
  out.append(',');
}

void format_node_with_indent(
    const AbstractNodeRepository &nr,
    const TokenFormatter &tf,
    AbstractString &out,
    size_t node_id,
    int indent
);

void print_node_field(
    const AbstractNodeRepository &nr,
    const TokenFormatter &tf,
    AbstractString &out,
    Text name,
    NodeId node_id,
    int indent
) {
  open_line(out, indent);
  out.append(name);
  out.append('=');
  format_node_with_indent(nr, tf, out, node_id, indent);
}

void print_node_field_with_comma(
    const AbstractNodeRepository &nr,
    const TokenFormatter &tf,
    AbstractString &out,
    Text name,
    NodeId node_id,
    int indent
) {
  print_node_field(nr, tf, out, name, node_id, indent);
  out.append(',');
}

void print_node_list_field(
    const AbstractNodeRepository &nr,
    const TokenFormatter &tf,
    AbstractString &out,
    Text name,
    ConstSlice<NodeId> nodes,
    int indent
) {
  open_line(out, indent);
  out.append(name);
  out.append("=[");
  if (nodes.size() > 0) {
    for (size_t i = 0; i < nodes.size(); ++i) {
      open_line(out, indent + 2);
      format_node_with_indent(nr, tf, out, nodes[i], indent + 2);
      if (i < nodes.size() - 1) {
        out.append(',');
      }
    }
    open_line(out, indent);
  }
  out.append(']');
}

void format_node_with_indent(
    const AbstractNodeRepository &nr,
    const TokenFormatter &tf,
    AbstractString &out,
    size_t node_id,
    int indent
) {
  const Node &generic_node = nr.get_node(node_id);
  node_type_to_string(out, generic_node.type());
  out.append('(');
  if (generic_node.type() == NodeType::IdentifierNode) {
    const auto &node = generic_node.as_IdentifierNode();
    print_token_field(tf, out, "name", node.name, indent + 2);
  } else if (generic_node.type() == NodeType::LetStatementNode) {
    const auto &node = generic_node.as_LetStatementNode();
    print_node_field_with_comma(nr, tf, out, "target", node.target, indent + 2);
    print_node_field(nr, tf, out, "expression", node.expression, indent + 2);
  } else if (generic_node.type() == NodeType::ConstStatementNode) {
    const auto &node = generic_node.as_ConstStatementNode();
    print_node_field_with_comma(nr, tf, out, "target", node.target, indent + 2);
    print_node_field(nr, tf, out, "expression", node.expression, indent + 2);
  } else if (generic_node.type() == NodeType::StringLiteralNode) {
    const auto &node = generic_node.as_StringLiteralNode();
    print_token_field(tf, out, "value", node.value, indent + 2);
  } else if (generic_node.type() == NodeType::NumberLiteralNode) {
    const auto &node = generic_node.as_NumberLiteralNode();
    print_token_field(tf, out, "value", node.value, indent + 2);
  } else if (generic_node.type() == NodeType::ParenthesizedExpressionNode) {
    const auto &node = generic_node.as_ParenthesizedExpressionNode();
    print_node_list_field(nr, tf, out, "exprs", node.exprs.data(), indent + 2);
  } else if (generic_node.type() == NodeType::ModuleNode) {
    const auto &node = generic_node.as_ModuleNode();
    print_node_list_field(nr, tf, out, "decls", node.decls.data(), indent + 2);
  } else if (generic_node.type() == NodeType::ArrayLiteralNode) {
    const auto &node = generic_node.as_ArrayLiteralNode();
    print_node_list_field(nr, tf, out, "exprs", node.exprs.data(), indent + 2);
  } else if (generic_node.type() == NodeType::BlockExpressionNode) {
    const auto &node = generic_node.as_BlockExpressionNode();
    print_node_list_field(nr, tf, out, "stmts", node.stmts.data(), indent + 2);
  } else if (generic_node.type() == NodeType::ObjectLiteralNode) {
    const auto &node = generic_node.as_ObjectLiteralNode();
    print_node_list_field(nr, tf, out, "entries", node.entries.data(), indent + 2);
  } else if (generic_node.type() == NodeType::KeyValueEntryNode) {
    const auto &node = generic_node.as_KeyValueEntryNode();
    print_node_field_with_comma(nr, tf, out, "key", node.key, indent + 2);
    print_node_field(nr, tf, out, "value", node.value, indent + 2);
  } else if (generic_node.type() == NodeType::ExpressionStatementNode) {
    const auto &node = generic_node.as_ExpressionStatementNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::IfThenExpressionNode) {
    const auto &node = generic_node.as_IfThenExpressionNode();
    print_node_field_with_comma(nr, tf, out, "condition", node.condition, indent + 2);
    print_node_field(nr, tf, out, "then_branch", node.then_branch, indent + 2);
  } else if (generic_node.type() == NodeType::IfThenElseExpressionNode) {
    const auto &node = generic_node.as_IfThenElseExpressionNode();
    print_node_field_with_comma(nr, tf, out, "condition", node.condition, indent + 2);
    print_node_field_with_comma(nr, tf, out, "then_branch", node.then_branch, indent + 2);
    print_node_field(nr, tf, out, "else_branch", node.else_branch, indent + 2);
  } else if (generic_node.type() == NodeType::CatchClauseNode) {
    const auto &node = generic_node.as_CatchClauseNode();
    print_node_field_with_comma(nr, tf, out, "exc_type", node.exc_type, indent + 2);
    print_node_field(nr, tf, out, "body", node.body, indent + 2);
  } else if (generic_node.type() == NodeType::CatchClauseBindingNode) {
    const auto &node = generic_node.as_CatchClauseBindingNode();
    print_token_field_with_comma(tf, out, "var", node.var, indent + 2);
    print_node_field_with_comma(nr, tf, out, "exc_type", node.exc_type, indent + 2);
    print_node_field(nr, tf, out, "body", node.body, indent + 2);
  } else if (generic_node.type() == NodeType::TryCatchExpressionNode) {
    const auto &node = generic_node.as_TryCatchExpressionNode();
    print_node_field_with_comma(nr, tf, out, "try_block", node.try_block, indent + 2);
    print_node_list_field(nr, tf, out, "clauses", node.clauses.data(), indent + 2);
  } else if (generic_node.type() == NodeType::CaseClauseNode) {
    const auto &node = generic_node.as_CaseClauseNode();
    print_node_field_with_comma(nr, tf, out, "expr", node.expr, indent + 2);
    print_node_field(nr, tf, out, "body", node.body, indent + 2);
  } else if (generic_node.type() == NodeType::SwitchExpressionNode) {
    const auto &node = generic_node.as_SwitchExpressionNode();
    print_node_field_with_comma(nr, tf, out, "expr", node.expr, indent + 2);
    print_node_list_field(nr, tf, out, "clauses", node.clauses.data(), indent + 2);
  } else if (generic_node.type() == NodeType::OrExpressionNode) {
    const auto &node = generic_node.as_OrExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::AndExpressionNode) {
    const auto &node = generic_node.as_AndExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::BitwiseOrExpressionNode) {
    const auto &node = generic_node.as_BitwiseOrExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::BitwiseXorExpressionNode) {
    const auto &node = generic_node.as_BitwiseXorExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::BitwiseAndExpressionNode) {
    const auto &node = generic_node.as_BitwiseAndExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::EqualsExpressionNode) {
    const auto &node = generic_node.as_EqualsExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::NotEqualsExpressionNode) {
    const auto &node = generic_node.as_NotEqualsExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::GreaterEqualsExpressionNode) {
    const auto &node = generic_node.as_GreaterEqualsExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::LessEqualsExpressionNode) {
    const auto &node = generic_node.as_LessEqualsExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::GreaterExpressionNode) {
    const auto &node = generic_node.as_GreaterExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::LessExpressionNode) {
    const auto &node = generic_node.as_LessExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::LeftShiftExpressionNode) {
    const auto &node = generic_node.as_LeftShiftExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::RightShiftExpressionNode) {
    const auto &node = generic_node.as_RightShiftExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::AddExpressionNode) {
    const auto &node = generic_node.as_AddExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::SubtractExpressionNode) {
    const auto &node = generic_node.as_SubtractExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::MultiplyExpressionNode) {
    const auto &node = generic_node.as_MultiplyExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::DivideExpressionNode) {
    const auto &node = generic_node.as_DivideExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::ModuloExpressionNode) {
    const auto &node = generic_node.as_ModuloExpressionNode();
    print_node_field_with_comma(nr, tf, out, "left", node.left, indent + 2);
    print_node_field(nr, tf, out, "right", node.right, indent + 2);
  } else if (generic_node.type() == NodeType::RefExpressionNode) {
    const auto &node = generic_node.as_RefExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::AwaitExpressionNode) {
    const auto &node = generic_node.as_AwaitExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::NotExpressionNode) {
    const auto &node = generic_node.as_NotExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::BitwiseNotExpressionNode) {
    const auto &node = generic_node.as_BitwiseNotExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::DerefExpressionNode) {
    const auto &node = generic_node.as_DerefExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::PositiveExpressionNode) {
    const auto &node = generic_node.as_PositiveExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::NegativeExpressionNode) {
    const auto &node = generic_node.as_NegativeExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::EllipsisExpressionNode) {
    const auto &node = generic_node.as_EllipsisExpressionNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::FieldAccessExpressionNode) {
    const auto &node = generic_node.as_FieldAccessExpressionNode();
    print_node_field_with_comma(nr, tf, out, "object", node.object, indent + 2);
    print_node_field(nr, tf, out, "field", node.field, indent + 2);
  } else if (generic_node.type() == NodeType::NumericFieldAccessExpressionNode) {
    const auto &node = generic_node.as_NumericFieldAccessExpressionNode();
    print_node_field_with_comma(nr, tf, out, "object", node.object, indent + 2);
    print_token_field(tf, out, "field", node.field, indent + 2);
  } else if (generic_node.type() == NodeType::IndexingExpressionNode) {
    const auto &node = generic_node.as_IndexingExpressionNode();
    print_node_field_with_comma(nr, tf, out, "object", node.object, indent + 2);
    print_node_field(nr, tf, out, "index", node.index, indent + 2);
  } else if (generic_node.type() == NodeType::PositionalFunctionArgumentNode) {
    const auto &node = generic_node.as_PositionalFunctionArgumentNode();
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::NamedFunctionArgumentNode) {
    const auto &node = generic_node.as_NamedFunctionArgumentNode();
    print_node_field_with_comma(nr, tf, out, "name", node.name, indent + 2);
    print_node_field(nr, tf, out, "expr", node.expr, indent + 2);
  } else if (generic_node.type() == NodeType::FunctionCallExpressionNode) {
    const auto &node = generic_node.as_FunctionCallExpressionNode();
    print_node_field_with_comma(nr, tf, out, "callee", node.callee, indent + 2);
    print_node_list_field(nr, tf, out, "args", node.args.data(), indent + 2);
  } else {
    throw RuntimeError("Unknown node type");
  }
  open_line(out, indent);
  out.append(')');
}

} // namespace

void NodeFormatter::format_node(AbstractString &out, size_t node_id) const {
  format_node_with_indent(m_node_repo, m_token_formatter, out, node_id, 0);
}

} // namespace amelia
