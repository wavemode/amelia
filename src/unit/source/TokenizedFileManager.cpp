#include "TokenizedFileManager.h"
#include "Prelude.h"

#include "data/source/Token.h"

namespace amelia {

TokenizedFileId TokenizedFileManager::store_tokenized_file(List<Token> tokens) {
  source_files.emplace_back(std::move(tokens));
  return source_files.size() - 1;
}

const List<Token> &TokenizedFileManager::get_tokenized_file(TokenizedFileId id) {
  return source_files[id];
}

} // namespace amelia
