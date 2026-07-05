#pragma once

#include "parser/data/node_type_list.hpp"

namespace amelia {

class ParserResult;
struct LexerResult;

struct Parser {
  static NodeId parse_module(ParserResult &output, const LexerResult &input);
};

} // namespace amelia
