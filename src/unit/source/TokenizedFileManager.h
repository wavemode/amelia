#pragma once

#include <cstddef>
#include <vector>

#include "interface/source/ITokenizedFileManager.h"

namespace amelia {

class TokenizedFileManager : public ITokenizedFileManager {
public:
  tokenized_file_id store_tokenized_file(std::vector<Token>) override;
  const std::vector<Token> &get_tokenized_file(tokenized_file_id) override;

private:
  std::vector<std::vector<Token>> source_files;
};

} // namespace amelia
