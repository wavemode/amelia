#pragma once

#include "prelude.hpp"

#include "data/lexer/lexer_result.hpp"
#include "data/parser/parser_result.hpp"
#include "data/sema/module.hpp"
#include "data/util/flex_shared.hpp"
#include "data/util/map.hpp"

namespace amelia {

struct SemaResult {
  Map<Text, ModuleId> module_ids;
  List<FlexShared<Module>> modules;
  List<LexerResult> module_tokens;
  List<ParserResult> module_syntax_trees;
  List<List<ModuleId>> module_imports;
  List<List<ModuleId>> modules_imported_by;
  List<List<ModuleId>> module_group_ids;
  List<List<List<Binding *>>> module_binding_deps;
};

} // namespace amelia
