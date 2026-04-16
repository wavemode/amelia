#pragma once

#include <vector>

#include "interface/lexer/ILexer.h"

namespace amelia {

class Token;
class Text;
class LexerContext;

class Lexer : public ILexer {
public:
  void tokenize(LexerContext ctx, Text input, std::vector<Token> &output) override;
};

} // namespace amelia
