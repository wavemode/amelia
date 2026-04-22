#include <doctest.h>

#include "data/core/List.h"
#include "data/core/String.h"
#include "data/core/Text.h"
#include "data/lexer/LexerContext.h"
#include "data/source/Token.h"
#include "unit/lexer/Lexer.h"

TEST_SUITE_BEGIN("Lexer");

using namespace amelia;

TEST_CASE("basic tokenization test") {
  Lexer lexer;
  List<Token> tokens;

  lexer.tokenize({.filename = "test_file"}, "x = y", tokens);
  CHECK(tokens.size() == 4);
  CHECK(tokens[0].type == TokenType::IDENTIFIER);
  CHECK(tokens[0].contents == "x");
  CHECK(tokens[0].location.filename == "test_file");
  CHECK(tokens[0].location.line == 1);
  CHECK(tokens[0].location.column == 1);

  CHECK(tokens[1].type == TokenType::ASSIGN);
  CHECK(tokens[1].contents == "=");
  CHECK(tokens[1].location.filename == "test_file");
  CHECK(tokens[1].location.line == 1);
  CHECK(tokens[1].location.column == 3);

  CHECK(tokens[2].type == TokenType::IDENTIFIER);
  CHECK(tokens[2].contents == "y");
  CHECK(tokens[2].location.filename == "test_file");
  CHECK(tokens[2].location.line == 1);
  CHECK(tokens[2].location.column == 5);

  CHECK(tokens[3].type == TokenType::END_OF_FILE);
  CHECK(tokens[3].contents == "");
  CHECK(tokens[3].location.filename == "test_file");
  CHECK(tokens[3].location.line == 1);
  CHECK(tokens[3].location.column == 6);
}

TEST_SUITE_END();
