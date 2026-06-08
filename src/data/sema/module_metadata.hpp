#pragma once

#include "prelude.hpp"

#include "data/util/set.hpp"

#include "data/lexer/lexer_result.hpp"
#include "data/parser/parser_result.hpp"

namespace amelia {

using BindingId = int32_t;

struct ModuleMetadata {
  String source;
  LexerResult tokens;
  ParserResult ast;
  NodeId ast_root;
  Set<ModuleId> imports;
  Set<ModuleId> imported_by;
  Set<ModuleId> group_module_ids;
  List<Set<BindingId>> binding_deps;
};

} // namespace amelia
