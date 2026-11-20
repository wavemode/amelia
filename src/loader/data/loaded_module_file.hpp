#pragma once

#include "lexer/data/lexer_result.hpp"
#include "loader/data/loaded_module_details.hpp"
#include "parser/data/parser_result.hpp"
#include "util/data/file_path.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleFile {
  FilePath path;
  String text_contents;
  LexerResult tokens;
  ParserResult ast;
  LoadedModuleDetails details;
};

} // namespace amelia
