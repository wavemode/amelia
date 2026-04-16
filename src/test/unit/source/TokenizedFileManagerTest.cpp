#include <doctest.h>

#include "data/source/Token.h"
#include "unit/source/TokenizedFileManager.h"

#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("TokenizedFileManager");

using namespace amelia;

TEST_CASE("can store and retrieve source file") {
  TokenizedFileManager manager;
  std::vector<Token> tokens = {Token{TokenType::IDENTIFIER, Location{1, 1, 1}, "hello"}};

  tokenized_file_id id = manager.store_tokenized_file(tokens);
  const std::vector<Token> &retrieved = manager.get_tokenized_file(id);
  CHECK(tokens == retrieved);
}

TEST_SUITE_END();
