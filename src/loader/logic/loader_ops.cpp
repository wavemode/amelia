#include "loader_ops.hpp"

#include "loader/data/loaded_module_details.hpp"
#include "loader/data/loaded_module_submodule_decl.hpp"
#include "loader/data/loader_result.hpp"
#include "loader/data/loader_settings.hpp"
#include "parser/data/node_location_error.hpp"
#include "parser/logic/ast_ops.hpp"
#include "util/data/text_utils.hpp"
#include "util/interface/file_loader.hpp"

namespace amelia {

namespace {

struct LoaderContext {
  Ref<LoadedModuleDetails> module_details;
  Ref<LoadedModuleFile> module_file;
  Ref<LoaderResult> loader_result;
  Ref<IFileLoader> file_loader;
  Ref<LoaderSettings> settings;

  NodeId unwrap_annotations(NodeId node_id) {
    const Node *node = &get_node(node_id);
    while (node->type() == NodeType::AnnotationNode) {
      node_id = node->as_AnnotationNode().stmt;
      node = &get_node(node_id);
    }
    return node_id;
  }

  NodeId unwrap_visibility(NodeId node_id) {
    const Node *node = &get_node(node_id);
    if (node->type() == NodeType::VisibilityNode) {
      node_id = node->as_VisibilityNode().decl;
    }
    return node_id;
  }

  void process_import_items(NodeId decl_node_id) {
    const Node *node = &get_node(decl_node_id);
    LoadedModuleDeclarationVisibility visibility;
    if (node->type() == NodeType::VisibilityNode) {
      process_visibility(visibility, decl_node_id);
      node = &get_node(node->as_VisibilityNode().decl);
    }

    for (NodeId item_node_id : node->as_ImportDeclNode().items.value()) {
      auto item = Box<LoadedModuleImportDeclItem>::emplace();
      item->visibility = visibility;
    }
  }

  void process_module_import(NodeId decl_node_id) {
    throw RuntimeError("not implemented");
  }

  void process_import_decl(NodeId decl_node_id) {
    decl_node_id = unwrap_annotations(decl_node_id);
    const auto &node =
        get_node(unwrap_visibility(decl_node_id)).as_ImportDeclNode();
    if (node.items.has_value()) {
      process_import_items(decl_node_id);
    } else {
      process_module_import(decl_node_id);
    }
  }

  void process_module_name_decl(NodeId node_id) {
    throw RuntimeError("not implemented");
    // // set name decl
    // module_details->name_decl = node_id;
    //
    // // process visibility
    // const Node *node = &get_node(node_id);
    // if (node->type() == NodeType::VisibilityNode) {
    //   process_visibility(module_details->visibility, node_id);
    //   node = &get_node(node->as_VisibilityNode().decl);
    // }
    //
    // // validate name
    // String name;
    // get_identifier_text(name, module_file->ast,
    //                     node->as_ModuleNode().name.value());
    // if (name != module_details->name) {
    //   throw NodeLocationError(
    //       node_id, "Module name declaration does not match file name");
    // }
  }

  void process_top_level_item_decl(NodeId node_id) {
    throw RuntimeError("not implemented");
  }

  void process_visibility(LoadedModuleDeclarationVisibility &result,
                          NodeId node_id) {
    const auto &node = get_node(node_id).as_VisibilityNode();
    result.visibility = node.visibility;
    if (node.scope.has_value()) {
      String &scope = result.scope.emplace();
      get_identifier_text(scope, module_file->ast, node.scope.value());
    }
  }

  void process_submodule_decl(NodeId node_id) {
    node_id = unwrap_annotations(node_id);

    // Box<LoadedModuleSubmoduleDecl>::emplace(module_file);

    // create submodule decl Box
    auto decl =
        Box<LoadedModuleSubmoduleDecl>(LoadedModuleSubmoduleDecl(module_file));
    decl->details.parent_module = module_details;

    // process visibility
    const Node *node = &get_node(node_id);
    if (node->type() == NodeType::VisibilityNode) {
      process_visibility(decl->visibility, node_id);
      node_id = node->as_VisibilityNode().decl;
      node = &get_node(node_id);
    }
    // process name
    get_identifier_text(decl->name, module_file->ast,
                        node->as_ModuleNode().name.value());

    // process decls
    auto ctx = *this;
    ctx.module_details = decl->details;
    ctx.process_top_level_decls(node_id);

    // add box to module_details
    String decl_name = decl->name;
    module_details->decls.set(move(decl_name),
                              decl.move_as<AbstractLoadedModuleDecl>());
  }

  void process_top_level_decls(NodeId root_node_id) {
    root_node_id = unwrap_annotations(root_node_id);
    const Node *node = &get_node(root_node_id);

    if (node->type() == NodeType::VisibilityNode) {
      process_visibility(module_details->visibility, root_node_id);
      node = &get_node(node->as_VisibilityNode().decl);
    }

    // TODO: check that decls appear in the order of imports -> module name ->
    // items

    for (NodeId decl_node_id : node->as_ModuleNode().decls.value()) {
      const Node *decl_node = &get_node(decl_node_id);
    check_node_type:
      switch (decl_node->type()) {
      case NodeType::VisibilityNode:
        node = &get_node(node->as_VisibilityNode().decl);
        goto check_node_type;
      case NodeType::ModuleNode:
        if (node->as_ModuleNode().decls.has_value()) {
          process_submodule_decl(root_node_id);
        } else {
          // TODO: error if this is a submodule
          process_module_name_decl(root_node_id);
        }
        break;
      case NodeType::ImportDeclNode:
        process_import_decl(decl_node_id);
        break;
      default:
        process_top_level_item_decl(decl_node_id);
        break;
      }
    }
  }

  const Node &get_node(NodeId node_id) {
    return module_file->ast.get_node(node_id);
  }
};

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
