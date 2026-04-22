#pragma once

#include <cstddef>

namespace amelia {

class Token;
using TokenizedFileId = size_t;

template <typename T> class List;

class ITokenizedFileManager {
public:
  virtual TokenizedFileId store_tokenized_file(List<Token>) = 0;
  virtual const List<Token> &get_tokenized_file(TokenizedFileId) = 0;
};

} // namespace amelia
