#include "ast_ops.hpp"

#include "lexer/data//lexer.hpp"
#include "parser/data/node_location_error.hpp"
#include "parser/data/parser_result.hpp"
#include "util/data/abstract_string.hpp"
#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

namespace {

void get_single_identifier_text(AbstractString &output, const ParserResult &ast,
                                NodeId node_id) {
  const Node &n = ast.get_node(node_id);
  if (n.type() != NodeType::IdentifierNode) {
    throw NodeLocationError(node_id, "Expected an identifier");
  }
  const IdentifierNode &node = n.as_IdentifierNode();
  Lexer::read_string_literal_contents(output, node.contents, node.raw);
}

} // namespace

void get_identifier_text(AbstractString &output, const ParserResult &ast,
                         NodeId node_id) {
  List<String> parts;
  const Node *node = &ast.get_node(node_id);
  while (node->type() == NodeType::ScopeResolutionExprNode) {
    const auto &scope_resolution_node = node->as_ScopeResolutionExprNode();
    String &part = parts.emplace_back();
    get_single_identifier_text(part, ast, scope_resolution_node.name);
    node_id = scope_resolution_node.scope;
    node = &ast.get_node(node_id);
  }
  String &part = parts.emplace_back();
  get_single_identifier_text(part, ast, node_id);

  for (size_t i = parts.size(); i > 0; --i) {
    output.append(parts[i - 1]);
    if (i > 1) {
      output.append("::");
    }
  }
}

} // namespace amelia
