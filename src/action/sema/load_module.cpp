#include "load_module.hpp"

#include "data/util/text_utils.hpp"

#include "data/lexer/lexer.hpp"
#include "data/parser/parser.hpp"
#include "data/source/source_location_error.hpp"

namespace amelia {

namespace {

Text unquote_identifier(Text ident) {
  if (TextUtils::starts_with(ident, "`")) {
    return TextUtils::slice_bytes(ident, 1, ident.size() - 1);
  }
  return ident;
}

void build_module_path(String &output, Text base_path, Slice<Text> module_parts) {
  Text path_separator = TextUtils::determine_path_separator(base_path);
  output.append(base_path);
  if (!TextUtils::ends_with(base_path, path_separator)) {
    output.append(path_separator);
  }
  for (size_t index = 0; index < module_parts.size(); ++index) {
    output.append(unquote_identifier(module_parts[index]));
    if (index < module_parts.size() - 1) {
      output.append(path_separator);
    }
  }
  output.append(".am");
}

void collect_imports(Module &module_obj) {
  const auto &module_node = module_obj.ast.get_node(module_obj.ast_root).as_ModuleNode();
  for (NodeId import_node_id : module_node.imports) {
    const Node &import_node = module_obj.ast.get_node(import_node_id);
    List<Text> result;
    NodeId import_path_node_id = import_node.as_ImportDeclNode().path;
    while (true) {
      const Node &import_path_node = module_obj.ast.get_node(import_path_node_id);
      if (import_path_node.type() == NodeType::IdentifierNode) {
        result.push_back(import_path_node.as_IdentifierNode().name);
        break;
      }

      const auto &scope_resolution = import_path_node.as_ScopeResolutionExprNode();
      const auto &scope_resolution_name = module_obj.ast.get_node(scope_resolution.name);

      if (scope_resolution_name.type() != NodeType::IdentifierNode) {
        throw SourceLocationError(
            module_obj.tokens.get_token(scope_resolution_name.start_token()).location,
            "Expected identifier in import statement"
        );
      }

      result.push_back(scope_resolution_name.as_IdentifierNode().name);
      result.push_back("::");
      import_path_node_id = scope_resolution.scope;
    }

    String output;
    for (int i = result.size() - 1; i >= 0; --i) {
      output.append(result[i]);
    }
    module_obj.imports.push_back(
        ModuleImport{move(output), module_obj.tokens.get_token(import_node.start_token()).location}
    );
  }
}

void collect_submodules(Module &module_obj, Text base_module, const List<NodeId> &submodules) {
  for (NodeId submodule_node_id : submodules) {
    const auto &module_decl = module_obj.ast.get_node(submodule_node_id).as_ModuleDeclNode();
    const auto &module_name = module_obj.ast.get_node(module_decl.name);
    const Token &module_name_token = module_obj.tokens.get_token(module_name.start_token());
    Text module_name_text = module_name.as_IdentifierNode().name;

    String result;
    result.append(base_module);
    result.append("::");
    result.append(module_name_text);
    if (module_obj.submodules.has(result)) {
      String error_message = "Duplicate submodule name '";
      error_message.append(result);
      error_message.append("'");
      throw SourceLocationError(module_name_token.location, move(error_message));
    }
    module_obj.submodules.set(result, module_name_token.location);

    collect_submodules(module_obj, result, module_decl.submodules.value());
  }
}

void collect_submodules(Module &module_obj, Text base_module) {
  const auto &module_node = module_obj.ast.get_node(module_obj.ast_root).as_ModuleNode();
  collect_submodules(module_obj, base_module, module_node.submodules);
}

void mark_as_loaded(
    SemaResult &sema_result,
    const String &module_name,
    const String &module_contents,
    const String &module_path,
    ModuleId module_id,
    Option<Location> submodule_location,
    Option<Location> import_location
) {
  Option<ModuleId> maybe_existing_module_id = sema_result.module_ids.find(module_name);
  if (maybe_existing_module_id.has_value()) {
    ModuleId existing_module_id = maybe_existing_module_id.value();
    Module &existing_module_obj = sema_result.modules[existing_module_id];
    Option<Location> existing_submodule_location;
    if (existing_module_obj.name != module_name) {
      existing_submodule_location = existing_module_obj.submodules.get(module_name);
    }

    if (existing_module_obj.source != module_contents) {
      // If modules with the name name were defined in two different files with different
      // contents, then this module name is ambiguous and we must raise an error.
      String error_message = "Module '";
      error_message.append(module_name);
      error_message.append("' defined in multiple locations (");
      error_message.append(existing_module_obj.source_path);
      if (existing_submodule_location.has_value()) {
        // existing_module is not the conflicting module - it contains it.
        // so, we'll print the line and column where the conflicting submodule is declared
        const Location &loc = existing_submodule_location.value();
        error_message.append(":");
        TextUtils::to_string(error_message, loc.line);
        error_message.append(":");
        TextUtils::to_string(error_message, loc.column);
      }
      error_message.append(", ");
      error_message.append(module_path);
      if (submodule_location.has_value()) {
        // the new module is not the conflicting module - it contains it.
        // so, we'll print the line and column where the conflicting submodule is declared
        const Location &loc = submodule_location.value();
        error_message.append(":");
        TextUtils::to_string(error_message, loc.line);
        error_message.append(":");
        TextUtils::to_string(error_message, loc.column);
      }
      error_message.append(")");
      if (import_location.has_value()) {
        throw SourceLocationError(import_location.value(), move(error_message));
      }
      throw RuntimeError(error_message.c_str());
    }
  }
  sema_result.module_ids.set(module_name, module_id);
}

Option<ModuleId> try_load_and_parse(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const String &path,
    const String &module_name,
    Option<Location> import_location
) {
  String source;
  auto err = file_loader.try_load_file(source, path);
  if (err.has_value()) {
    return None();
  }
  ModuleId module_id = sema_result.modules.size();
  Module &module_obj = sema_result.modules.emplace_back();
  module_obj.analyzed = false;
  module_obj.name = module_name;
  module_obj.scope = Flex<Scope>::emplace();
  module_obj.source_path = path;
  module_obj.source = move(source);
  Lexer::tokenize(module_obj.tokens, {module_obj.source_path}, module_obj.source);
  module_obj.ast_root = Parser::parse_module(module_obj.ast, module_obj.tokens);
  collect_imports(module_obj);
  collect_submodules(module_obj, module_name);
  mark_as_loaded(
      sema_result,
      module_name,
      module_obj.source,
      module_obj.source_path,
      module_id,
      None(),
      import_location
  );
  for (const auto &[submodule_name, submodule_location] : module_obj.submodules) {
    mark_as_loaded(
        sema_result,
        submodule_name,
        module_obj.source,
        module_obj.source_path,
        module_id,
        submodule_location,
        import_location
    );
  }
  return Some(module_id);
}

ModuleId load_module(
    IFileLoader &file_loader,
    SemaResult &sema_result,
    const String &module_name,
    const ModuleLoaderContext &ctx,
    Set<Text> &import_set,
    List<Text> &import_chain,
    Option<Location> import_location = None()
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
          sema_result.modules[m].group_module_ids.add(g);
          // modules that import circularly are not considered dependencies of each other
          sema_result.modules[m].imported_ids.remove(g);
          sema_result.modules[m].imported_by_ids.remove(g);
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
  Option<ModuleId> target_module_id;
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
          file_loader, sema_result, module_path, loaded_module_name, import_location
      );
      if (try_loaded_module_id.has_value()) {
        ModuleId loaded_module_id = try_loaded_module_id.value();
        Module &loaded_module_obj = sema_result.modules[loaded_module_id];

        // check whether this is the module we were originally looking for
        if (loaded_module_name == module_name) {
          target_module_id = loaded_module_id;
        }

        if (!target_module_id.has_value()) {
          // check whether any submodule is the module we were originally looking for
          for (const auto &[loaded_submodule_name, loaded_module_location] :
               loaded_module_obj.submodules) {
            if (loaded_submodule_name == module_name) {
              target_module_id = loaded_module_id;
            }
          }
        }

        size_t prior_group_id_count = loaded_module_obj.group_module_ids.size();

        // load all of this module's imports
        import_set.add(loaded_module_name.text());
        import_chain.push_back(loaded_module_name.text());
        for (const ModuleImport &loaded_module_import : loaded_module_obj.imports) {

          ModuleId imported_module_id = load_module(
              file_loader,
              sema_result,
              loaded_module_import.name,
              ctx,
              import_set,
              import_chain,
              loaded_module_import.location
          );
          Module &imported_module_obj = sema_result.modules[imported_module_id];
          if (!loaded_module_obj.group_module_ids.has(imported_module_id)) {
            imported_module_obj.imported_by_ids.add(loaded_module_id);
            loaded_module_obj.imported_ids.add(imported_module_id);
          }
        }
        import_set.remove(loaded_module_name);
        import_chain.pop_back();

        // make sure all of our group modules are also in a group with each other
        if (loaded_module_obj.group_module_ids.size() > prior_group_id_count) {
          for (ModuleId group_module_id : loaded_module_obj.group_module_ids) {
            Module &group_module_obj = sema_result.modules[group_module_id];
            for (ModuleId other_group_module_id : loaded_module_obj.group_module_ids) {
              if (group_module_id != other_group_module_id) {
                group_module_obj.group_module_ids.add(other_group_module_id);
                // modules that import circularly are not considered dependencies of each other
                group_module_obj.imported_ids.remove(other_group_module_id);
                group_module_obj.imported_by_ids.remove(other_group_module_id);
              }
            }
          }
        }
      }
      module_name_parts.pop_back();
    }
  }

  if (target_module_id.has_value()) {
    return target_module_id.value();
  }

  String error_message = "Could not find module '";
  error_message.append(module_name);
  error_message.append("' in module path");
  if (import_location.has_value()) {
    throw SourceLocationError(import_location.value(), move(error_message));
  }
  throw RuntimeError(error_message.c_str());
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
  load_module(file_loader, sema_result, module_name, ctx, import_set, import_chain);
}

} // namespace amelia
