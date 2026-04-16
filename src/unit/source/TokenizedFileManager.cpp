#include "TokenizedFileManager.h"

#include "data/source/Token.h"

namespace amelia {

tokenized_file_id TokenizedFileManager::store_tokenized_file(std::vector<Token> tokens) {
  source_files.emplace_back(std::move(tokens));
  return source_files.size() - 1;
}

const std::vector<Token> &TokenizedFileManager::get_tokenized_file(tokenized_file_id id) {
  return source_files[id];
}

} // namespace amelia
