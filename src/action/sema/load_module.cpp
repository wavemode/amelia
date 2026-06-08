#include "load_module.hpp"

#include "data/util/text_utils.hpp"

#include "data/lexer/lexer.hpp"
#include "data/parser/parser.hpp"
#include "data/source/source_location_error.hpp"

namespace amelia {

namespace {

Text determine_path_separator(Text path) {
  if (TextUtils::find(path, "\\").at_end()) {
    return "/";
  }
  return "\\";
}

void build_module_path(String &output, Text base_path, Slice<Text> module_parts) {
  Text path_separator = determine_path_separator(base_path);
  output.append(base_path);
  if (!TextUtils::ends_with(base_path, path_separator)) {
    output.append(path_separator);
  }
  TextUtils::join_into(output, module_parts, path_separator);
  output.append(".am");
}

Text identifier_name(const Token &name) {
  switch (name.type) {
  case TokenType::IDENTIFIER:
  case TokenType::IDENTIFIER_NO_W:
    return name.contents;
  case TokenType::QUOTED_IDENTIFIER:
  case TokenType::QUOTED_IDENTIFIER_NO_W:
    return TextUtils::substr_bytes(name.contents, 1, name.contents.size() - 1);
  default:
    throw SourceLocationError(name.location, "Expected identifier in import statement");
  }
}

struct ModuleImport {
  String name;
  Location location;
};

void collect_imports(List<ModuleImport> &imports, const ModuleMetadata &module_meta) {
  const auto &module_node = module_meta.ast.get_node(module_meta.ast_root).as_ModuleNode();
  for (NodeId import_node_id : module_node.imports) {
    const Node &import_node = module_meta.ast.get_node(import_node_id);
    List<Text> result;
    NodeId import_path_node_id = import_node.as_ImportDeclNode().path;
    while (true) {
      const Node &import_path_node = module_meta.ast.get_node(import_path_node_id);
      if (import_path_node.type() == NodeType::IdentifierNode) {
        auto token_id = import_path_node.as_IdentifierNode().token;
        const auto &token = module_meta.tokens.get_token(token_id);
        Text name_part = identifier_name(token);
        result.push_back(name_part);
        break;
      }

      const auto &scope_resolution = import_path_node.as_ScopeResolutionExprNode();
      const auto &scope_resolution_name = module_meta.ast.get_node(scope_resolution.name);

      if (scope_resolution_name.type() != NodeType::IdentifierNode) {
        throw SourceLocationError(
            module_meta.tokens.get_token(scope_resolution_name.start_token()).location,
            "Expected identifier in import statement"
        );
      }

      auto token_id = scope_resolution_name.as_IdentifierNode().token;
      const auto &token = module_meta.tokens.get_token(token_id);
      Text name_part = identifier_name(token);
      result.push_back(name_part);
      result.push_back("::");
      import_path_node_id = scope_resolution.scope;
    }

    String output;
    for (int i = result.size() - 1; i >= 0; --i) {
      output.append(result[i]);
    }
    imports.push_back(
        ModuleImport{move(output), module_meta.tokens.get_token(import_node.start_token()).location}
    );
  }
}

void collect_submodules(
    List<String> &output,
    const ModuleMetadata &module_meta,
    Text base_module,
    const List<NodeId> &submodules
) {
  for (NodeId submodule_node_id : submodules) {
    const auto &module_decl = module_meta.ast.get_node(submodule_node_id).as_ModuleDeclNode();
    const auto &module_name = module_meta.ast.get_node(module_decl.name).as_IdentifierNode();
    const Token &module_name_token = module_meta.tokens.get_token(module_name.token);
    Text module_name_text = identifier_name(module_name_token);

    String &result = output.emplace_back();
    result.append(base_module);
    result.append("::");
    result.append(module_name_text);

    collect_submodules(output, module_meta, result, module_decl.submodules.value());
  }
}

void collect_submodules(List<String> &output, const ModuleMetadata &module_meta, Text base_module) {
  const auto &module_node = module_meta.ast.get_node(module_meta.ast_root).as_ModuleNode();
  collect_submodules(output, module_meta, base_module, module_node.submodules);
}

Option<ModuleId> try_load_and_parse(
    IFileLoader &file_loader, SemaResult &sema_result, const String &path
) {
  String source;
  auto err = file_loader.try_load_file(source, path);
  if (err.has_value()) {
    return None();
  }
  ModuleId module_id = sema_result.module_meta.size();
  ModuleMetadata &module_meta = sema_result.module_meta.emplace_back();
  module_meta.source = move(source);
  Lexer::tokenize(module_meta.tokens, {path}, module_meta.source);
  module_meta.ast_root = Parser::parse_module(module_meta.ast, module_meta.tokens);
  return Some(module_id);
}

Option<ModuleId> load_module(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const String &module_name,
    const ModuleLoaderContext &ctx,
    Set<Text> &import_set,
    List<Text> &import_chain
) {
  // check if this module is involved in a chain of circular imports
  if (import_set.has(module_name)) {
    List<ModuleId> group_ids;
    int index = import_chain.size() - 1;
    while (true) {
      const Text &m = import_chain[index];
      group_ids.push_back(sema_result.module_ids[String(m)]);
      if (m == module_name) {
        break;
      }
      --index;
    }
    for (ModuleId m : group_ids) {
      for (ModuleId g : group_ids) {
        if (m != g) {
          sema_result.module_meta[m].group_module_ids.add(g);
        }
      }
    }
  }

  // check if this module has already been loaded
  Option<ModuleId> existing_module_id = sema_result.module_ids.find(module_name);
  if (existing_module_id.has_value()) {
    return existing_module_id.value();
  }

  // load the module
  String loaded_module_name;
  for (const String &base_path : ctx.module_path) {
    List<Text> module_name_parts;
    TextUtils::split(module_name_parts, module_name, "::");
    while (module_name_parts.size() > 0) {
      String module_path;
      build_module_path(module_path, base_path, module_name_parts.data());

      loaded_module_name.clear();
      TextUtils::join_into(loaded_module_name, module_name_parts.data(), "::");

      Option<ModuleId> try_loaded_module_id = try_load_and_parse(
          file_loader, sema_result, module_path
      );
      if (try_loaded_module_id.has_value()) {
        ModuleId loaded_module_id = try_loaded_module_id.value();
        bool found_target_module = loaded_module_name == module_name;

        // mark this module as loaded
        sema_result.module_ids.set(loaded_module_name, loaded_module_id);
        sema_result.modules.push_back(FlexShared<Module>::strong(Module{loaded_module_name, Scope{}}
        ));

        // mark all of this module's submodules as loaded
        ModuleMetadata &loaded_module_meta = sema_result.module_meta[loaded_module_id];
        List<String> loaded_submodule_list;
        collect_submodules(loaded_submodule_list, loaded_module_meta, loaded_module_name);
        for (const String &loaded_submodule_name : loaded_submodule_list) {
          sema_result.module_ids.set(loaded_submodule_name, loaded_module_id);
          if (!found_target_module && loaded_submodule_name == module_name) {
            found_target_module = true;
          }
        }

        // load all of this module's imports
        List<ModuleImport> loaded_module_imports;
        collect_imports(loaded_module_imports, loaded_module_meta);

        import_set.add(loaded_module_name.text());
        import_chain.push_back(loaded_module_name.text());
        for (const ModuleImport &loaded_module_import : loaded_module_imports) {
          Option<ModuleId> try_imported_module_id = load_module(
              file_loader, sema_result, loaded_module_import.name, ctx, import_set, import_chain
          );
          if (!try_imported_module_id.has_value()) {
            String error_message = "Could not find module '";
            error_message.append(loaded_module_import.name);
            error_message.append("' in module path");
            throw SourceLocationError(loaded_module_import.location, move(error_message));
          }
          ModuleId imported_module_id = try_imported_module_id.value();
          ModuleMetadata &imported_module_meta = sema_result.module_meta[imported_module_id];
          imported_module_meta.imported_by.add(loaded_module_id);
          loaded_module_meta.imports.add(imported_module_id);
        }
        import_set.remove(loaded_module_name);
        import_chain.pop_back();

        if (found_target_module) {
          return loaded_module_id;
        }
      }
      module_name_parts.pop_back();
    }
  }
  return None();
}

} // namespace

void load_module(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const String &module_name,
    const ModuleLoaderContext &ctx
) {
  Set<Text> import_set;
  List<Text> import_chain;
  if (!load_module(file_loader, sema_result, module_name, ctx, import_set, import_chain)
           .has_value()) {
    String error_message = "Could not find module '";
    error_message.append(module_name);
    error_message.append("' in module path");
    throw RuntimeError(error_message.c_str());
  };
}

} // namespace amelia
