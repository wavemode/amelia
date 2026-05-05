#pragma once

namespace amelia {

class Text;
struct LexerContext;
struct LexerResult;

struct Lexer {
  static void tokenize(LexerResult &output, LexerContext ctx, Text input);
};

} // namespace amelia
