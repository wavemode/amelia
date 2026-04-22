#pragma once

#include "data/core/List.h"

namespace amelia {

class Token;
class Text;
class LexerContext;
template <typename T> class List;

class ILexer {
public:
  virtual void tokenize(LexerContext ctx, Text input, List<Token> &output) = 0;
};

} // namespace amelia
