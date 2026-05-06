#pragma once

#include <cstddef>

namespace amelia {

class ParserResult;
class LexerResult;
using NodeId = size_t;

struct Parser {
  static NodeId parse_module(ParserResult &output, const LexerResult &input);
};

} // namespace amelia
