#pragma once

#include "prelude.hpp"

#include "testing/data/serialize.hpp"
#include "util/data/map.hpp"
#include "util/data/set.hpp"

#include "lexer/data/lexer_result.hpp"
#include "parser/data/parser_result.hpp"
#include "sema/data/scope.hpp"

namespace amelia {

struct ModuleImport {
  String name;
  Location location;
};

struct Module {
  String name;
  String source_path;
  String source;
  LexerResult tokens;
  ParserResult ast;
  NodeId ast_root;
  Flex<Scope> scope;
  List<ModuleImport> imports;
  Map<String, Location> submodules;
  Set<ModuleId> imported_ids;
  Set<ModuleId> imported_by_ids;
  Set<ModuleId> group_module_ids;
  List<Set<BindingId>> binding_deps;
  bool analyzed;

  Serialize serialize() const;
};

} // namespace amelia
