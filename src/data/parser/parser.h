#pragma once

#include "data/parser/node_type_list.h"

namespace amelia {

class ParserResult;
class LexerResult;

struct Parser {
  static NodeId parse_module(ParserResult &output, const LexerResult &input);
};

} // namespace amelia
