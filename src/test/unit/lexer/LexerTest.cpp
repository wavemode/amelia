#include <doctest.h>

#include "unit/lexer/Lexer.h"
#include "data/source/Token.h"
#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("Lexer");

TEST_CASE("dummy") {
  amelia::Lexer lexer;
  std::vector<amelia::Token> tokens;
  CHECK_THROWS_AS(lexer.tokenize("", tokens), std::runtime_error);
}

TEST_SUITE_END();
