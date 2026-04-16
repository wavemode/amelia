#include "Lexer.h"

#include <cstddef>

#include "data/lexer/LexerContext.h"
#include "util/text/Text.h"

namespace amelia {

namespace {

struct LexerState {
  LexerContext ctx;
  size_t line;
  size_t column;
  size_t position;
  Text input;
  std::vector<Token> &output;
};

} // namespace

void Lexer::tokenize(LexerContext ctx, Text input, std::vector<Token> &output) {
  LexerState state{ctx, 1, 1, 0, input, output};

  throw std::runtime_error("not implemented");
}

} // namespace amelia
