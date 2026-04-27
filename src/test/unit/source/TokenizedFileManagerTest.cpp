#include "Prelude.h"
#include <doctest.h>

#include "data/source/Token.h"
#include "unit/source/TokenizedFileManager.h"

TEST_SUITE_BEGIN("TokenizedFileManager");

using namespace amelia;

TEST_CASE("can store and retrieve source file") {
  TokenizedFileManager manager;
  List<Token> tokens = {
      Token{TokenType::IDENTIFIER, Location{"dummy_file", CharIterator(Text()), 1, 1}, "hello"}
  };

  TokenizedFileId id = manager.store_tokenized_file(tokens);
  const List<Token> &retrieved = manager.get_tokenized_file(id);
  CHECK(tokens == retrieved);
}

TEST_SUITE_END();
