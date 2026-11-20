#include "load_module.hpp"

#include "lexer/data/lexer.hpp"
#include "loader/data/loaded_module_import_decl.hpp"
#include "loader/data/loaded_module_import_item.hpp"
#include "loader/data/loaded_module_item_decl.hpp"
#include "loader/data/loaded_module_submodule_decl.hpp"
#include "loader/data/loaded_module_wildcard_import.hpp"
#include "loader/data/loader_result.hpp"
#include "loader/data/loader_settings.hpp"
#include "parser/data/parser.hpp"
#include "parser/logic/ast_ops.hpp"
#include "resolver/data/resolved_module_import.hpp"
#include "source/data/module_name.hpp"
#include "source/data/source_location_error.hpp"
#include "util/interface/file_loader.hpp"

namespace amelia {

namespace {

struct LoaderContext {
  LoaderContext(LoaderResult &loader_result, IFileLoader &file_loader)
      : loader_result(loader_result), file_loader(file_loader) {}

  Ref<LoaderResult> loader_result;
  Ref<IFileLoader> file_loader;

  Option<Ref<LoadedModuleDetails>> module_details;
  Option<Ref<LoadedModuleFile>> module_file;

  List<NodeId> decl_annotations;
  List<NodeId> decl_names;
  Option<LoadedDeclarationVisibility> decl_visibility;

  void try_load_module_by_name(const ModuleName &module_name) {
    for (const FilePath &search_path : loader_result->settings.search_path) {
      FilePath working_path = search_path;
      for (const auto &module_name_part : module_name.parts()) {
        working_path.add_part(module_name_part);
      }
      working_path.set_extension(".am");
      ModuleName working_module_name = module_name;
      while (working_module_name.parts().size() > 0) {
        try_load_file_by_path(working_path, working_module_name);
        working_path.pop_part();
        working_module_name.pop_part();
      }
    }
  }

  void try_load_file_by_path(const FilePath &path,
                             const ModuleName &module_name) {
    if (loader_result->loaded_files.has(path)) {
      return;
    }

    String text_contents;
    if (auto err = file_loader->try_load_file(text_contents, path);
        err.has_value()) {
      return;
    }

    auto loaded_file = Box<LoadedModuleFile>::emplace();
    loaded_file->path = path;
    loaded_file->tokens.set_filename(FilePath(path));
    loaded_file->text_contents = move(text_contents);
    Lexer::tokenize(loaded_file->tokens, loaded_file->text_contents.text());
    auto root_decl_node_id =
        Parser::parse_module(loaded_file->ast, loaded_file->tokens);
    loaded_file->details.name = module_name;
    loaded_file->details.short_name =
        loaded_file->details.name
            .parts()[loaded_file->details.name.num_parts() - 1];
    loaded_file->details.file = *loaded_file;

    LoadedModuleFile &loaded_file_ref = *loaded_file;
    loader_result->loaded_files.set(path, move(loaded_file));

    auto ctx = *this;
    ctx.module_file = loaded_file_ref;
    ctx.module_details = loaded_file_ref.details;
    ctx.process_top_level_decls(root_decl_node_id);
  }

  void process_top_level_decls(NodeId root_decl_node_id) {
    const auto &module_node = get_node(root_decl_node_id).as_ModuleNode();
    bool seen_module_name = false, seen_item_decl = false;
    for (NodeId decl_node_id : module_node.decls.value()) {
      const auto &decl_node = get_node(unwrap_node(decl_node_id));
      if (decl_node.type() == NodeType::ModuleNode) {
        const auto &submodule_decl_node = decl_node.as_ModuleNode();
        if (!submodule_decl_node.decls.has_value()) {
          // if this is a module name decl, error if item decls have been seen
          if (seen_item_decl) {
            throw_error_at_node_id(decl_node_id,
                                   "Module name declaration must come before "
                                   "any item declarations");
          } else if (seen_module_name) {
            throw_error_at_node_id(decl_node_id,
                                   "Multiple module name declarations in the "
                                   "same file");
          } else if (module_details.value()->parent.has_value()) {
            throw_error_at_node_id(
                decl_node_id,
                "Module name declaration not allowed in submodule");
          }
          seen_module_name = true;
        } else {
          seen_item_decl = true;
        }
        process_module_decl(decl_node_id, submodule_decl_node);
      } else if (decl_node.type() == NodeType::ImportDeclNode) {
        const auto &import_decl_node = decl_node.as_ImportDeclNode();
        if (seen_item_decl || seen_module_name) {
          throw_error_at_node_id(
              decl_node_id,
              "Import declarations must come first in the module");
        }
        process_import_decl(decl_node_id, import_decl_node);
      } else {
        seen_item_decl = true;
        process_item_decl(decl_node_id);
      }
    }
  }

  void process_import_decl(NodeId import_decl_node_id,
                           const ImportDeclNode &import_decl_node) {
    auto import_decl = Box<LoadedModuleImportDecl>::emplace();

    if (decl_visibility.has_value()) {
      import_decl->visibility = move(decl_visibility).value();
    }
    if (decl_annotations.size() > 0) {
      import_decl->annotations = move(decl_annotations);
    }

    // get module name and load corresponding file
    String module_name_str;
    get_text(module_name_str, import_decl_node.module);
    import_decl->module = ModuleName(module_name_str);
    try {
      import_decl->module.validate();
    } catch (const RuntimeError &e) {
      throw_error_at_node_id(import_decl_node_id, String(e.message()));
    }
    try_load_module_by_name(import_decl->module);

    // process items
    if (import_decl_node.items.has_value()) {
      import_decl->items = List<Box<AbstractLoadedModuleImportItem>>();
      for (NodeId item_node_id : import_decl_node.items.value()) {
        const Node &item_node = get_node(item_node_id);
        if (item_node.type() == NodeType::ImportItemNode) {
          const auto &import_item_node = item_node.as_ImportItemNode();
          auto import_item = Box<LoadedModuleImportItem>::emplace();

          get_text(import_item->name, import_item_node.name);

          if (import_item_node.alias.has_value()) {
            import_item->alias = String();
            get_text(import_item->alias.value(),
                     import_item_node.alias.value());
          }

          import_decl->items.value().push_back(move(import_item));
        } else {
          // only remaining possibility is a wildcard node
          import_decl->items.value().push_back(
              Box<LoadedModuleWildcardImport>::emplace());
        }
      }
    }

    // process alias if present
    if (import_decl_node.alias.has_value()) {
      import_decl->alias = String();
      get_text(import_decl->alias.value(), import_decl_node.alias.value());
    }

    module_details.value()->imports.push_back(move(import_decl));
  }

  void process_module_decl(NodeId module_decl_node_id,
                           const ModuleNode &module_node) {
    if (module_node.decls.has_value()) {
      // submodule declaration
      auto submodule_decl = Box<LoadedModuleSubmoduleDecl>::emplace();
      get_text(submodule_decl->details.short_name, module_node.name.value());
      submodule_decl->details.name = module_details.value()->name;
      submodule_decl->details.name.add_part(submodule_decl->details.short_name);
      submodule_decl->details.parent = module_details.value();
      submodule_decl->details.file = module_file.value();
      if (decl_visibility.has_value()) {
        submodule_decl->details.visibility = move(decl_visibility).value();
      }
      if (decl_annotations.size() > 0) {
        submodule_decl->details.annotations = move(decl_annotations);
      }

      auto ctx = *this;
      ctx.module_details = submodule_decl->details;
      ctx.process_top_level_decls(module_decl_node_id);

      module_details.value()->submodules.set(
          submodule_decl->details.short_name.text(), move(submodule_decl));
    } else {
      String declared_name;
      get_text(declared_name, module_node.name.value());
      if (declared_name != module_details.value()->short_name) {
        throw_error_at_node_id(module_decl_node_id,
                               "Declared module name does not match file name");
      }
      if (decl_visibility.has_value()) {
        module_details.value()->visibility = move(decl_visibility).value();
      }
      if (decl_annotations.size() > 0) {
        module_details.value()->annotations = move(decl_annotations);
      }
    }
  }

  void process_item_decl(NodeId module_item_decl_node_id) {
    auto item_decl = Box<LoadedModuleItemDecl>::emplace();
    item_decl->node_id = module_item_decl_node_id;
    if (decl_visibility.has_value()) {
      item_decl->visibility = move(decl_visibility).value();
    }
    for (NodeId name_node_id : decl_names) {
      String &name = item_decl->names.emplace_back();
      get_text(name, name_node_id);
    }
    if (decl_annotations.size() > 0) {
      item_decl->annotations = move(decl_annotations);
    }
    module_details.value()->items.push_back(move(item_decl));
  }

  NodeId unwrap_node(NodeId node_id) {
    // unwrap and record all annotations
    decl_annotations.clear();

    const Node *node = &get_node(node_id);
    while (node->type() == NodeType::AnnotationNode) {
      const auto &annotation_node = node->as_AnnotationNode();
      decl_annotations.push_back(node_id);
      node_id = annotation_node.stmt;
      node = &get_node(node_id);
    }

    // then unwrap and record visibility
    decl_visibility = None();

    if (node->type() == NodeType::VisibilityNode) {
      const auto &visibility_node = node->as_VisibilityNode();
      decl_visibility = LoadedDeclarationVisibility();
      decl_visibility.value().visibility = visibility_node.visibility;
      if (visibility_node.scope.has_value()) {
        decl_visibility.value().scope = String();
        get_text(decl_visibility.value().scope.value(),
                 visibility_node.scope.value());
      }
      node_id = visibility_node.decl;
    }

    // then extract name based on node type
    decl_names.clear();

    read_decl_name(node_id);

    // then return core node id
    return node_id;
  }

  void read_decl_name(NodeId node_id) {
    const Node &node = get_node(node_id);
    switch (node.type()) {
    case NodeType::ClassDeclNode:
      decl_names.push_back(node.as_ClassDeclNode().name);
      break;
    case NodeType::RecordDeclNode:
      read_decl_name(node.as_RecordDeclNode().decl);
      break;
    case NodeType::AbstractDeclNode:
      read_decl_name(node.as_AbstractDeclNode().decl);
      break;
    case NodeType::OpenDeclNode:
      read_decl_name(node.as_OpenDeclNode().decl);
      break;
    case NodeType::FunctionDeclNode:
      decl_names.push_back(node.as_FunctionDeclNode().name);
      break;
    case NodeType::AsyncDeclNode:
      read_decl_name(node.as_AsyncDeclNode().decl);
      break;
    case NodeType::ConceptDeclNode:
      decl_names.push_back(node.as_ConceptDeclNode().name);
      break;
    case NodeType::ConstDeclNode:
      for (NodeId init_node_id : node.as_ConstDeclNode().inits) {
        read_decl_name(init_node_id);
      }
      break;
    case NodeType::LetDeclNode:
      for (NodeId init_node_id : node.as_LetDeclNode().inits) {
        read_decl_name(init_node_id);
      }
      break;
    case NodeType::InitializerNode:
      read_decl_name(node.as_InitializerNode().pattern);
      break;
    case NodeType::StructPatternNode:
      for (NodeId field_node_id : node.as_StructPatternNode().elements) {
        read_decl_name(field_node_id);
      }
      break;
    case NodeType::StructFieldPatternNode:
      if (node.as_StructFieldPatternNode().pattern.has_value()) {
        read_decl_name(node.as_StructFieldPatternNode().pattern.value());
      } else {
        read_decl_name(node.as_StructFieldPatternNode().field);
      }
      break;
    case NodeType::TuplePatternNode:
      for (NodeId element_node_id : node.as_TuplePatternNode().elements) {
        read_decl_name(element_node_id);
      }
      break;
    case NodeType::ArrayPatternNode:
      for (NodeId element_node_id : node.as_ArrayPatternNode().elements) {
        read_decl_name(element_node_id);
      }
      break;
    case NodeType::IdentifierPatternNode:
      decl_names.push_back(node.as_IdentifierPatternNode().identifier);
      break;
    case NodeType::UnderscorePatternNode:
      break;
    case NodeType::ImplicitDeclNode:
      decl_names.push_back(node.as_ImplicitDeclNode().name);
      break;
    case NodeType::EnumDeclNode:
      decl_names.push_back(node.as_EnumDeclNode().name);
      break;
    case NodeType::TypeDeclNode:
      decl_names.push_back(node.as_TypeDeclNode().name);
      break;
    case NodeType::UnionDeclNode:
      decl_names.push_back(node.as_UnionDeclNode().name);
      break;
    case NodeType::ModuleNode:
      decl_names.push_back(node.as_ModuleNode().name.value());
      break;
    case NodeType::ImportDeclNode:
      break;
    default:
      throw_error_at_node_id(node_id, "Invalid top-level declaration");
    }
  }

  const Node &get_node(NodeId node_id) {
    return module_file.value()->ast.get_node(node_id);
  }

  void get_text(AbstractString &output, NodeId node_id) {
    try {
      return get_identifier_text(output, module_file.value()->ast, node_id);
    } catch (const RuntimeError &err) {
      throw_error_at_node_id(node_id, String(err.message()));
    }
  }

  [[noreturn]] void throw_error_at_node_id(NodeId node_id, String &&message) {
    const Node &node = get_node(node_id);
    const Token &start_token =
        module_file.value()->tokens.get_token(node.start_token());
    throw SourceLocationError(start_token.location, move(message));
  }
};

} // namespace

void load_module(LoaderResult &result, IFileLoader &file_loader,
                 const ModuleName &module_name) {
  module_name.validate();
  LoaderContext(result, file_loader).try_load_module_by_name(module_name);
}

} // namespace amelia
