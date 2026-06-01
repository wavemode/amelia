#pragma once

#include <cstddef>

#include "data/lexer/abstract_token_repository.hpp"
#include "data/lexer/token_formatter.hpp"
#include "data/parser/abstract_node_repository.hpp"
#include "data/parser/node.hpp"
#include "prelude.hpp"

namespace amelia {

class NodeFormatter {
public:
  NodeFormatter(const AbstractNodeRepository &node_repo, const AbstractTokenRepository &token_repo);

  void format_node(AbstractString &out, NodeId node_id);

private:
  void print_node_field(AbstractString &out, Text name, const List<NodeId> &nodes_value);

  void print_node_field(AbstractString &out, Text name, const Option<List<NodeId>> &nodes);

  void print_node_field(AbstractString &out, Text name, Option<NodeId> node_id);

  void print_token_field(AbstractString &out, Text name, TokenId token_id);

  void print_field(AbstractString &out, Text name, Text value);

  void print_field(AbstractString &out, Text name, bool value);

  void open_line(AbstractString &out, bool with_comma = true);

  void print_indent(AbstractString &out) const;

  const AbstractTokenRepository &m_token_repo;
  const TokenFormatter m_token_formatter;
  const AbstractNodeRepository &m_node_repo;
  int m_fields_printed;
  int m_current_indent;
};

} // namespace amelia
