#pragma once

#include <cstdint>

namespace amelia {

using NodeId = int32_t;
struct AbstractNodeRepository;
struct AbstractString;
class Text;
template <typename T> class List;
template <typename T> class Option;

class NodeFormatter {
public:
  NodeFormatter(const AbstractNodeRepository &node_repo);

  void format_node(AbstractString &out, NodeId node_id);

private:
  void print_node_field(AbstractString &out, Text name, const List<NodeId> &nodes_value);

  void print_node_field(AbstractString &out, Text name, const Option<List<NodeId>> &nodes);

  void print_node_field(AbstractString &out, Text name, Option<NodeId> node_id);

  void print_field(AbstractString &out, Text name, Text value);

  void print_field(AbstractString &out, Text name, bool value);

  void open_line(AbstractString &out, bool with_comma = true);

  void print_indent(AbstractString &out) const;

  const AbstractNodeRepository &m_node_repo;
  uint32_t m_fields_printed;
  uint32_t m_current_indent;
};

} // namespace amelia
