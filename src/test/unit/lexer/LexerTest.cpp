#include <doctest.h>

#include "data/source/Token.h"
#include "unit/lexer/Lexer.h"
#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("Lexer");

using namespace amelia;

TEST_CASE("dummy") {
  Lexer lexer;
  std::vector<Token> tokens;
  CHECK_THROWS_AS(lexer.tokenize("", tokens), std::runtime_error);
}

TEST_SUITE_END();
