#pragma once

#include <cstdint>

namespace amelia {

using NodeId = int32_t;
class ParserResult;
struct LexerResult;

struct Parser {
  static NodeId parse_module(ParserResult &output, const LexerResult &input);
};

} // namespace amelia
