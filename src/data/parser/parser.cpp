#include "parser.h"
#include "prelude.h"

#include "data/lexer/lexer_result.h"
#include "data/parser/parser_context.h"
#include "data/parser/parser_result.h"

namespace amelia {

namespace {

class ParserState {
public:
  ParserState(ParserResult &output, ParserContext ctx, const LexerResult &input)
      : output(output), ctx(ctx), input(input), index(0) {}

private:
  ParserResult &output;
  ParserContext ctx;
  const LexerResult &input;
  size_t index;
};
} // namespace

void Parser::parse(ParserResult &output, ParserContext ctx, const LexerResult &input) {}

} // namespace amelia
