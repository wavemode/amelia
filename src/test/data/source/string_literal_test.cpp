#include <doctest.h>

#include "data/source/string_literal.h"
#include "prelude.h"

TEST_SUITE_BEGIN("StringLiteral");

using namespace amelia;

TEST_CASE("parsing") {
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
      {"\r", ""},
      {"\\\nhi", "hi"},
      {"\\\r\nhi", "hi"},
      {"\\\rhi", "hi"},
      {"", ""},
  };

  std::vector<TestCase> raw_test_cases = {
      {R"(Raw string with no escapes: \n\t\\)", R"(Raw string with no escapes: \n\t\\)"},
      {R"(Raw string with "quotes" and \backslashes\)",
       R"(Raw string with "quotes" and \backslashes\)"},
  };

  for (const TestCase &test_case : test_cases) {
    CharIterator iter(test_case.input);
    String result;
    StringLiteral::read(result, iter, false);
    CHECK(result.text() == test_case.expected_output);
  }

  for (const TestCase &test_case : raw_test_cases) {
    CharIterator iter(test_case.input);
    String result;
    StringLiteral::read(result, iter, true);
    CHECK(result.text() == test_case.expected_output);
  }
}

TEST_SUITE_END();
