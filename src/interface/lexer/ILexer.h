#pragma once

#include <vector>

namespace amelia {

class Token;
class Text;
class LexerContext;

class ILexer {
public:
  virtual void tokenize(LexerContext ctx, Text input, std::vector<Token> &output) = 0;
};

} // namespace amelia
