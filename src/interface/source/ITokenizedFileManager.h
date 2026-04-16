#pragma once

#include <cstddef>
#include <vector>

namespace amelia {

class Token;
using tokenized_file_id = size_t;

class ITokenizedFileManager {
public:
  virtual tokenized_file_id store_tokenized_file(std::vector<Token>) = 0;
  virtual const std::vector<Token> &get_tokenized_file(tokenized_file_id) = 0;
};

} // namespace amelia
