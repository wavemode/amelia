#pragma once

#include <cstddef>
#include <vector>

#include "data/lexer/lexer_result.h"
#include "data/parser/node_info.h"
#include "data/util/text_utils.h"
#include "node_type_list.h"
#include "prelude.h"

namespace amelia {

class ParserResult {
public:
  NodeInfo get_node_info(size_t id) const {
    return m_nodes[id];
  }

  void format_node(AbstractString &out, const LexerResult &lr, size_t node_id) const {
    format_node_with_indent(out, lr, node_id, 0);
  }

private:
  void format_node_with_indent(
      AbstractString &out, const LexerResult &lr, size_t node_id, int indent
  ) const {
    NodeInfo info = get_node_info(node_id);
    node_type_to_string(out, info.type);
    out.append('(');
    if (info.type == NodeType::IdentifierNode) {
      const auto &node = m_IdentifierNode_nodes.get(node_id);
      print_token_field(out, lr, "name", node.name, indent + 2);
    } else if (info.type == NodeType::LetStatementNode) {
      const auto &node = m_LetStatementNode_nodes.get(node_id);
      print_node_field_with_comma(out, lr, "target", node.target, indent + 2);
      print_node_field(out, lr, "expression", node.expression, indent + 2);
    } else if (info.type == NodeType::ConstStatementNode) {
      const auto &node = m_ConstStatementNode_nodes.get(node_id);
      print_node_field_with_comma(out, lr, "target", node.target, indent + 2);
      print_node_field(out, lr, "expression", node.expression, indent + 2);
    } else if (info.type == NodeType::StringLiteralNode) {
      const auto &node = m_StringLiteralNode_nodes.get(node_id);
      print_token_field(out, lr, "value", node.value, indent + 2);
    } else if (info.type == NodeType::NumberLiteralNode) {
      const auto &node = m_NumberLiteralNode_nodes.get(node_id);
      print_token_field(out, lr, "value", node.value, indent + 2);
    } else if (info.type == NodeType::ParenthesizedExpressionNode) {
      const auto &node = m_ParenthesizedExpressionNode_nodes.get(node_id);
      print_node_list_field(out, lr, "exprs", node.exprs.data(), indent + 2);
    } else if (info.type == NodeType::ModuleNode) {
      const auto &node = m_ModuleNode_nodes.get(node_id);
      print_node_list_field(out, lr, "decls", node.decls.data(), indent + 2);
    } else if (info.type == NodeType::ArrayLiteralNode) {
      const auto &node = m_ArrayLiteralNode_nodes.get(node_id);
      print_node_list_field(out, lr, "exprs", node.exprs.data(), indent + 2);
    } else if (info.type == NodeType::BlockExpressionNode) {
      const auto &node = m_BlockExpressionNode_nodes.get(node_id);
      print_node_list_field(out, lr, "stmts", node.stmts.data(), indent + 2);
    } else if (info.type == NodeType::ObjectLiteralNode) {
      const auto &node = m_ObjectLiteralNode_nodes.get(node_id);
      print_entry_list_field(out, lr, "entries", node.entries.data(), indent + 2);
    }
    open_line(out, indent);
    out.append(')');
  }

  void print_entry_list_field(
      AbstractString &out,
      const LexerResult &lr,
      Text name,
      ConstSlice<KeyValueEntryNode> entries,
      int indent
  ) const {
    open_line(out, indent);
    out.append(name);
    out.append("=[");
    if (entries.size() > 0) {
      for (size_t i = 0; i < entries.size(); ++i) {
        const auto &entry = entries[i];
        open_line(out, indent + 2);
        lr.format_token(out, entry.field);
        out.append(" = ");
        format_node_with_indent(out, lr, entry.value, indent + 2);
        if (i < entries.size() - 1) {
          out.append(',');
        }
      }
      open_line(out, indent);
    }
    out.append(']');
  }

  void print_node_list_field(
      AbstractString &out, const LexerResult &lr, Text name, ConstSlice<NodeId> nodes, int indent
  ) const {
    open_line(out, indent);
    out.append(name);
    out.append("=[");
    if (nodes.size() > 0) {
      for (size_t i = 0; i < nodes.size(); ++i) {
        open_line(out, indent + 2);
        format_node_with_indent(out, lr, nodes[i], indent + 2);
        if (i < nodes.size() - 1) {
          out.append(',');
        }
      }
      open_line(out, indent);
    }
    out.append(']');
  }

  void print_node_field(
      AbstractString &out, const LexerResult &lr, Text name, NodeId node_id, int indent
  ) const {
    open_line(out, indent);
    out.append(name);
    out.append('=');
    format_node_with_indent(out, lr, node_id, indent);
  }

  void print_node_field_with_comma(
      AbstractString &out, const LexerResult &lr, Text name, NodeId node_id, int indent
  ) const {
    print_node_field(out, lr, name, node_id, indent);
    out.append(',');
  }

  void print_token_field(
      AbstractString &out, const LexerResult &lr, Text name, NodeId token_id, int indent
  ) const {
    open_line(out, indent);
    out.append(name);
    out.append('=');
    lr.format_token(out, token_id);
  }

  void print_token_field_with_comma(
      AbstractString &out, const LexerResult &lr, Text name, NodeId token_id, int indent
  ) const {
    print_token_field(out, lr, name, token_id, indent);
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

#define X(NODE_TYPE)                                                                               \
private:                                                                                           \
  Map<size_t, NODE_TYPE> m_##NODE_TYPE##_nodes;                                                    \
                                                                                                   \
public:                                                                                            \
  const NODE_TYPE &get_##NODE_TYPE(size_t id) const {                                              \
    return m_##NODE_TYPE##_nodes.get(id);                                                          \
  }                                                                                                \
                                                                                                   \
  size_t add_##NODE_TYPE(Location loc, NODE_TYPE node) {                                           \
    size_t id = add_node_info({loc, NodeType::NODE_TYPE});                                         \
    m_##NODE_TYPE##_nodes.set(id, std::move(node));                                                \
    return id;                                                                                     \
  }
  NODE_TYPE_LIST
#undef X

private:
  List<NodeInfo> m_nodes;
  size_t root_node_id = 0;

  size_t add_node_info(NodeInfo info) {
    m_nodes.push_back(info);
    return m_nodes.size() - 1;
  }
};

} // namespace amelia
