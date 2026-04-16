#pragma once

#include <vector>

namespace amelia {

class Token;
class Text;

class ILexer {
public:
  virtual void tokenize(Text input, std::vector<Token> &output) = 0;
};

} // namespace amelia
