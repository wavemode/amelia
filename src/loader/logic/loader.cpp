#include "loader.hpp"

#include "lexer/data/lexer.hpp"
#include "lexer/data/lexer_context.hpp"
#include "loader/data/loader_context.hpp"
#include "loader/data/loader_result.hpp"
#include "loader/data/source_file.hpp"
#include "loader/data/source_file_import.hpp"
#include "parser/data/parser.hpp"
#include "source/data/source_location_error.hpp"
#include "util/data/string.hpp"
#include "util/data/text_utils.hpp"
#include "util/interface/file_loader.hpp"

namespace amelia {

namespace {

Text unquote_identifier(Text ident) {
  if (TextUtils::starts_with(ident, "`")) {
    return TextUtils::slice_bytes(ident, 1, ident.size() - 1);
  }
  return ident;
}

void build_file_path(String &output, Text base_path, Slice<Text> module_parts) {
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

void collect_imports(SourceFile &source_file) {
  const auto &module_node = source_file.ast.get_node(source_file.ast_root).as_ModuleNode();
  for (NodeId import_node_id : module_node.imports) {
    const Node &import_node = source_file.ast.get_node(import_node_id);
    List<Text> result;
    NodeId import_path_node_id = import_node.as_ImportDeclNode().path;
    while (true) {
      const Node &import_path_node = source_file.ast.get_node(import_path_node_id);
      if (import_path_node.type() == NodeType::IdentifierNode) {
        result.push_back(import_path_node.as_IdentifierNode().name);
        break;
      }

      const auto &scope_resolution = import_path_node.as_ScopeResolutionExprNode();
      const auto &scope_resolution_name = source_file.ast.get_node(scope_resolution.name);

      if (scope_resolution_name.type() != NodeType::IdentifierNode) {
        throw SourceLocationError(
            source_file.tokens.get_token(scope_resolution_name.start_token()).location,
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
    source_file.imports.push_back(SourceFileImport{
        move(output), source_file.tokens.get_token(import_node.start_token()).location
    });
  }
}

void collect_submodules(SourceFile &source_file, Text base_module, const List<NodeId> &submodules) {
  for (NodeId submodule_node_id : submodules) {
    const auto &module_decl = source_file.ast.get_node(submodule_node_id).as_ModuleDeclNode();
    const auto &module_name = source_file.ast.get_node(module_decl.name);
    const Token &module_name_token = source_file.tokens.get_token(module_name.start_token());
    Text module_name_text = module_name.as_IdentifierNode().name;

    String result;
    result.append(base_module);
    result.append("::");
    result.append(module_name_text);
    if (source_file.submodules.has(result)) {
      String error_message = "Duplicate submodule name '";
      error_message.append(result);
      error_message.append("'");
      throw SourceLocationError(module_name_token.location, move(error_message));
    }
    source_file.submodules.set(result, module_name_token.location);

    collect_submodules(source_file, result, module_decl.submodules.value());
  }
}

void collect_submodules(SourceFile &source_file, Text base_module) {
  const auto &module_node = source_file.ast.get_node(source_file.ast_root).as_ModuleNode();
  collect_submodules(source_file, base_module, module_node.submodules);
}

void mark_as_loaded(
    LoaderResult &loader_result,
    const String &module_name,
    const String &file_contents,
    const String &file_path,
    size_t file_id,
    Option<Location> submodule_location,
    Option<Location> import_location
) {
  Option<size_t> maybe_existing_file_id = loader_result.file_id_by_module_name.find(module_name);
  if (maybe_existing_file_id.has_value()) {
    size_t existing_file_id = maybe_existing_file_id.value();
    SourceFile &existing_source_file = loader_result.loaded_files[existing_file_id];
    Option<Location> existing_submodule_location;
    if (existing_source_file.module_name != module_name) {
      existing_submodule_location = existing_source_file.submodules.get(module_name);
    }

    if (existing_source_file.source != file_contents) {
      // If modules with the same name were defined in two different files with different
      // contents, then this module name is ambiguous and we must raise an error.
      String error_message = "SourceFile '";
      error_message.append(module_name);
      error_message.append("' defined in multiple locations (");
      error_message.append(existing_source_file.source_path);
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
      error_message.append(file_path);
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
  loader_result.file_id_by_module_name.set(module_name, file_id);
}

Option<size_t> try_load_and_parse(
    IFileLoader &file_loader,
    LoaderResult &loader_result,
    const String &path,
    const String &module_name,
    Option<Location> import_location
) {
  String source;
  auto err = file_loader.try_load_file(source, path);
  if (err.has_value()) {
    return None();
  }
  size_t file_id = loader_result.loaded_files.size();
  SourceFile &souce_file = loader_result.loaded_files.emplace_back();
  souce_file.module_name = module_name;
  souce_file.source_path = path;
  souce_file.source = move(source);
  Lexer::tokenize(souce_file.tokens, {souce_file.source_path}, souce_file.source);
  souce_file.ast_root = Parser::parse_module(souce_file.ast, souce_file.tokens);
  collect_imports(souce_file);
  collect_submodules(souce_file, module_name);
  mark_as_loaded(
      loader_result,
      module_name,
      souce_file.source,
      souce_file.source_path,
      file_id,
      None(),
      import_location
  );
  for (const auto &[submodule_name, submodule_location] : souce_file.submodules) {
    mark_as_loaded(
        loader_result,
        submodule_name,
        souce_file.source,
        souce_file.source_path,
        file_id,
        submodule_location,
        import_location
    );
  }
  return Some(file_id);
}

size_t load_module(
    IFileLoader &file_loader,
    LoaderResult &sema_result,
    const String &module_name,
    const LoaderContext &ctx,
    Set<Text> &import_set,
    List<Text> &import_chain,
    Option<Location> import_location = None()
) {
  // check if this module is involved in a chain of circular imports
  if (import_set.has(module_name)) {
    List<size_t> group_ids;
    int index = import_chain.size() - 1;
    while (true) {
      const Text &m = import_chain[index];
      group_ids.push_back(sema_result.file_id_by_module_name[String(m)]);
      if (m == module_name) {
        break;
      }
      --index;
    }
    for (size_t m : group_ids) {
      for (size_t g : group_ids) {
        if (m != g) {
          auto &source_file = *sema_result.loaded_files[m];
          const auto &group_file = sema_result.loaded_files[g];
          source_file.group_files.add(group_file);
          // modules that import circularly are not considered dependencies of each other
          source_file.imported_files.remove(group_file);
          source_file.imported_by_files.remove(group_file);
        }
      }
    }
  }

  // check if this module has already been loaded
  Option<size_t> existing_file_id = sema_result.file_id_by_module_name.find(module_name);
  if (existing_file_id.has_value()) {
    return existing_file_id.value();
  }

  // load the module
  Option<size_t> target_file_id;
  String loaded_module_name;
  for (const String &base_path : ctx.module_path) {
    List<Text> module_name_parts;
    TextUtils::split(module_name_parts, module_name, "::");
    while (module_name_parts.size() > 0) {
      String file_path;
      build_file_path(file_path, base_path, module_name_parts.data());

      loaded_module_name.clear();
      TextUtils::join_into(loaded_module_name, module_name_parts.data(), "::");

      Option<size_t> try_loaded_file_id = try_load_and_parse(
          file_loader, sema_result, file_path, loaded_module_name, import_location
      );
      if (try_loaded_file_id.has_value()) {
        size_t loaded_file_id = try_loaded_file_id.value();
        auto &loaded_source_file = sema_result.loaded_files[loaded_file_id];

        // check whether this is the module we were originally looking for
        if (loaded_module_name == module_name) {
          target_file_id = loaded_file_id;
        }

        if (!target_file_id.has_value()) {
          // check whether any submodule is the module we were originally looking for
          for (const auto &[loaded_submodule_name, loaded_module_location] :
               loaded_source_file->submodules) {
            if (loaded_submodule_name == module_name) {
              target_file_id = loaded_file_id;
            }
          }
        }

        size_t prior_group_id_count = loaded_source_file->group_files.size();

        // load all of this module's imports
        import_set.add(loaded_module_name.text());
        import_chain.push_back(loaded_module_name.text());
        for (const SourceFileImport &loaded_source_file_import : loaded_source_file->imports) {
          size_t imported_file_id = load_module(
              file_loader,
              sema_result,
              loaded_source_file_import.name,
              ctx,
              import_set,
              import_chain,
              loaded_source_file_import.location
          );
          auto &imported_source_file = sema_result.loaded_files[imported_file_id];
          if (!loaded_source_file->group_files.has(imported_source_file)) {
            imported_source_file->imported_by_files.add(loaded_source_file);
            loaded_source_file->imported_files.add(imported_source_file);
          }
        }
        import_set.remove(loaded_module_name);
        import_chain.pop_back();

        // make sure all of our group modules are also in a group with each other
        if (loaded_source_file->group_files.size() > prior_group_id_count) {
          for (auto group_source_file : loaded_source_file->group_files) {
            for (auto &other_group_source_file : loaded_source_file->group_files) {
              if (group_source_file != other_group_source_file) {
                group_source_file->group_files.add(other_group_source_file);
                // modules that import circularly are not considered dependencies of each other
                group_source_file->imported_files.remove(other_group_source_file);
                group_source_file->imported_by_files.remove(other_group_source_file);
              }
            }
          }
        }
      }
      module_name_parts.pop_back();
    }
  }

  if (target_file_id.has_value()) {
    return target_file_id.value();
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
    LoaderResult &loader_result,
    const String &module_name,
    const LoaderContext &ctx
) {
  Set<Text> import_set;
  List<Text> import_chain;
  load_module(file_loader, loader_result, module_name, ctx, import_set, import_chain);
}

} // namespace amelia
