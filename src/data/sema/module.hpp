#pragma once

#include "prelude.hpp"

#include "data/util/map.hpp"
#include "data/util/set.hpp"

#include "data/lexer/lexer_result.hpp"
#include "data/parser/parser_result.hpp"
#include "data/sema/scope.hpp"

namespace amelia {

struct ModuleImport {
  String name;
  Location location;
};

struct Module {
  Module() : scope(FlexShared<Scope>::strong(Scope())) {}

  String name;
  String source_path;
  String source;
  LexerResult tokens;
  ParserResult ast;
  NodeId ast_root;
  FlexShared<Scope> scope;
  List<ModuleImport> imports;
  Map<String, Location> submodules;
  Set<ModuleId> imported_ids;
  Set<ModuleId> imported_by_ids;
  Set<ModuleId> group_module_ids;
  List<Set<BindingId>> binding_deps;
};

} // namespace amelia
