#include <doctest.h>

#include "data/lexer/LexerContext.h"
#include "data/source/Token.h"
#include "unit/lexer/Lexer.h"
#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("Lexer");

using namespace amelia;

TEST_CASE("dummy") {
  Lexer lexer;
  std::vector<Token> tokens;
  LexerContext ctx{"dummy_file"};

  CHECK_THROWS_AS(lexer.tokenize(ctx, "", tokens), std::runtime_error);
}

TEST_SUITE_END();
