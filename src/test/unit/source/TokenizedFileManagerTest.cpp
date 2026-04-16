#include <doctest.h>

#include "data/source/Token.h"
#include "unit/source/TokenizedFileManager.h"

#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("TokenizedFileManager");

TEST_CASE("can store and retrieve source file") {
  amelia::TokenizedFileManager manager;
  std::vector<amelia::Token> tokens = {
      amelia::Token{amelia::TokenType::IDENTIFIER, amelia::Location{1, 1, 1}, "hello"}};

  amelia::tokenized_file_id id = manager.store_tokenized_file(tokens);
  const std::vector<amelia::Token> &retrieved = manager.get_tokenized_file(id);
  CHECK(tokens == retrieved);
}

TEST_SUITE_END();
