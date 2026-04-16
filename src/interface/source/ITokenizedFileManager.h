#pragma once

#include <cstddef>
#include <vector>

namespace amelia {

class Token;
using TokenizedFileId = size_t;

class ITokenizedFileManager {
public:
  virtual TokenizedFileId store_tokenized_file(std::vector<Token>) = 0;
  virtual const std::vector<Token> &get_tokenized_file(TokenizedFileId) = 0;
};

} // namespace amelia
