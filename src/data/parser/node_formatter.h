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

  void format_node(AbstractString &out, NodeId node_id) const;

private:
  void format_node_with_indent(AbstractString &out, NodeId node_id, int indent) const;

  void print_node_list_field_with_comma(
      AbstractString &out, Text name, ConstSlice<NodeId> nodes, int indent
  ) const;

  void print_node_list_field(AbstractString &out, Text name, ConstSlice<NodeId> nodes, int indent)
      const;

  void print_node_field(AbstractString &out, Text name, Option<NodeId> node_id, int indent) const;

  void print_node_field_with_comma(
      AbstractString &out, Text name, Option<NodeId> node_id, int indent
  ) const;

  void print_token_field(AbstractString &out, Text name, TokenId token_id, int indent) const;

  void print_token_field_with_comma(AbstractString &out, Text name, TokenId token_id, int indent)
      const;

  void print_field(AbstractString &out, Text name, Text value, int indent) const;

  void print_field(AbstractString &out, Text name, bool value, int indent) const;

  void print_field_with_comma(AbstractString &out, Text name, Text value, int indent) const;

  static void open_line(AbstractString &out, int indent);

  static void open_line_comma(AbstractString &out, int indent);

  static void print_indent(AbstractString &out, int indent);

  const TokenFormatter m_token_formatter;
  const AbstractNodeRepository &m_node_repo;
};

} // namespace amelia
