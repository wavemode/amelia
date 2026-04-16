#pragma once

#include <vector>

#include "interface/lexer/ILexer.h"

namespace amelia {

class Token;
class Text;

class Lexer : public ILexer {
public:
  void tokenize(Text input, std::vector<Token> &output) override;
};

} // namespace amelia
