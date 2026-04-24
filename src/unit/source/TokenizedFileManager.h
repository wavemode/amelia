#pragma once

#include <cstddef>

#include "data/core/List.h"
#include "data/source/Token.h"
#include "interface/source/ITokenizedFileManager.h"

namespace amelia {

class TokenizedFileManager : public ITokenizedFileManager {
public:
  TokenizedFileId store_tokenized_file(List<Token>) override;
  const List<Token> &get_tokenized_file(TokenizedFileId) override;

private:
  List<List<Token>> source_files;
};

} // namespace amelia
