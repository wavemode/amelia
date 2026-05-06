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
      open_line(out, indent + 2);
      out.append("name=");
      lr.format_token(out, node.name);
      open_line(out, indent);
    } else if (info.type == NodeType::LetStatementNode) {
      const auto &node = m_LetStatementNode_nodes.get(node_id);
      open_line(out, indent + 2);
      out.append("target=");
      format_node_with_indent(out, lr, node.target, indent + 2);
      open_line_comma(out, indent + 2);
      out.append("expression=");
      format_node_with_indent(out, lr, node.expression, indent + 2);
      open_line(out, indent);
    } else if (info.type == NodeType::ModuleNode) {
      const auto &node = m_ModuleNode_nodes.get(node_id);
      open_line(out, indent + 2);
      out.append("decls=[");
      if (node.decls.size() > 0) {
        for (size_t i = 0; i < node.decls.size(); ++i) {
          open_line(out, indent + 4);
          format_node_with_indent(out, lr, node.decls[i], indent + 4);
          if (i < node.decls.size() - 1) {
            out.append(',');
          }
        }
        open_line(out, indent + 2);
      }
      out.append(']');
      open_line(out, indent);
    }
    out.append(')');
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
