#pragma once

#include "lexer/data/lexer_result.hpp"
#include "loader/data/loaded_module_details.hpp"
#include "parser/data/parser_result.hpp"
#include "util/data/string.hpp"

namespace amelia {

using NodeId = int32_t;

struct LoadedModuleFile {
  String path;
  String file_contents;
  LexerResult tokens;
  ParserResult ast;
  NodeId root_node_id = -1;
  LoadedModuleDetails details;
};

}; // namespace amelia
