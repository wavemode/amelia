#pragma once

namespace amelia {

class ParserContext;
class ParserResult;
class LexerResult;

struct Parser {
  static void parse(ParserResult &output, ParserContext ctx, const LexerResult &input);
};

} // namespace amelia
