#include <cstdint>

#include <vendor/doctest.h>

#include "data/lexer/TokenId.h"

TEST_SUITE_BEGIN("TokenId");

TEST_CASE("can be constructed from an unsigned integer") {
  amelia::TokenId token_id_1(42u);
  CHECK(token_id_1.hash() == std::hash<uint64_t>()(42u));
}

TEST_CASE("can be constructed from another TokenId") {
  amelia::TokenId token_id_1(42u);
  amelia::TokenId token_id_2(token_id_1);
  CHECK(token_id_2.hash() == std::hash<uint64_t>()(42u));
}

TEST_CASE("equality and inequality operators") {
  amelia::TokenId token_id_1(42u);
  amelia::TokenId token_id_2(42u);
  amelia::TokenId token_id_3(43u);

  CHECK(token_id_1 == token_id_2);
  CHECK(token_id_1 != token_id_3);
  CHECK(token_id_2 != token_id_3);
}

TEST_SUITE_END();
