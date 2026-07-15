#pragma once

#include <cstdint>

#include "lexer/data/lexer_result.hpp"
#include "parser/data/parser_result.hpp"
#include "sema/data/scope.hpp"
#include "source/data/location.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/set.hpp"
#include "util/data/string.hpp"

namespace amelia {

class Serialize;

using ModuleId = int32_t;

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
