#pragma once

#include <cstdint>

#include "lexer/data/lexer_result.hpp"
#include "loader/data/source_file_import.hpp"
#include "parser/data/parser_result.hpp"
#include "source/data/location.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/set.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct SourceFile {
  String module_name;
  Map<String, Location> submodules;

  String source_path;
  String source;
  LexerResult tokens;
  ParserResult ast;
  NodeId ast_root;

  List<SourceFileImport> imports;
  Set<Flex<SourceFile>> imported_files;
  Set<Flex<SourceFile>> imported_by_files;
  Set<Flex<SourceFile>> group_files;
};

} // namespace amelia
