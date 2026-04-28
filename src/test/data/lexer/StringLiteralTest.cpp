#include <doctest.h>

#include "Prelude.h"
#include "data/lexer/StringLiteral.h"

TEST_SUITE_BEGIN("StringLiteral");

using namespace amelia;

TEST_CASE("escape sequences") {
  struct TestCase {
    Text input;
    Text expected_output;
  };

  std::vector<TestCase> test_cases = {
      {R"(Hello, world!)", "Hello, world!"},
      {R"(Line 1\nLine 2)", "Line 1\nLine 2"},
      {R"(Tab\tSeparated)", "Tab\tSeparated"},
      {R"(Backslash\\Test)", "Backslash\\Test"},
      {R"(Quote\"Test)", "Quote\"Test"},
      {R"(Single Quote\'Test)", "Single Quote\'Test"},
      {R"(Hex\x41Test)", "HexATest"},
      {R"(Unicode\U0001F600Test)", "Unicode😀Test"},
      {R"(Unicode\u1234Test)", "UnicodeሴTest"},
      {R"(Multiline\nString\nTest)", "Multiline\nString\nTest"},
      {"\r\n", "\n"},
      {"", ""},
  };

  for (const TestCase &test_case : test_cases) {
    CharIterator iter(test_case.input);
    String result;
    StringLiteral::read(result, iter, false);
    CHECK(result.text() == test_case.expected_output);
  }
}

TEST_SUITE_END();
