#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace amelia {

template <typename T> class Slice;
class Token;
class Char;

class ILexer {
public:
  virtual void tokenize(Slice<Char> input, std::vector<Token> &output) = 0;
};

} // namespace amelia
