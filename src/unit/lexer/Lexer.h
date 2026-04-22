#pragma once

#include "data/core/List.h"

#include "interface/lexer/ILexer.h"

namespace amelia {

class Lexer : public ILexer {
public:
  void tokenize(LexerContext ctx, Text input, List<Token> &output) override;
};

} // namespace amelia
