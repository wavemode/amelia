#include "loader_ops.hpp"

#include "loader/data/loaded_module_details.hpp"
#include "loader/data/loader_result.hpp"
#include "loader/data/loader_settings.hpp"
#include "parser/data/node_location_error.hpp"
#include "parser/logic/ast_ops.hpp"
#include "util/data/text_utils.hpp"
#include "util/interface/file_loader.hpp"

namespace amelia {

namespace {

NodeId unwrap_annotations(const LoadedModuleFile &file, NodeId node_id) {
  const Node *node = &file.ast.get_node(node_id);
  while (node->type() == NodeType::AnnotationNode) {
    node_id = node->as_AnnotationNode().stmt;
    node = &file.ast.get_node(node_id);
  }
  return node_id;
}

void process_visibility(LoadedModuleDeclarationVisibility &result,
                        const LoadedModuleFile &file, NodeId node_id) {
  const auto &node = file.ast.get_node(node_id).as_VisibilityNode();
  result.visibility = node.visibility;
  if (node.scope.has_value()) {
    String &scope = result.scope.emplace();
    get_identifier_text(scope, file.ast, node.scope.value());
  }
}

void process_single_decl(LoadedModuleDetails &module_details,
                         NodeId decl_node_id) {
  throw RuntimeError("not implemented");
}

void process_module_name_decl(LoadedModuleDetails &module_details,
                              NodeId root_node_id) {
  throw RuntimeError("not implemented");
}

void process_decls(LoadedModuleDetails &module_details, NodeId root_node_id) {
  throw RuntimeError("not implemented");
}

} // namespace

void add_module_file(LoaderResult &result, Text search_path, Text file_name,
                     Text file_contents) {
  throw RuntimeError("not implemented");
}

void import_module(LoaderResult &result, Text module_name) {
  throw RuntimeError("not implemented");
}

void load_module(LoaderResult &result, IFileLoader &file_loader,
                 const LoaderSettings &settings, Text module_name) {
  throw RuntimeError("not implemented");
}

} // namespace amelia
