#include <cstdint>
#include <cstring>

#include <doctest.h>

#include "util/data/char_iterator.hpp"
#include "util/data/string.hpp"

TEST_SUITE_BEGIN("CharIterator");

using namespace amelia;

TEST_CASE("can iterate over a String - ASCII") {
  String str("Hello, world!");
  auto iter = str.begin();
  auto end = str.end();
  REQUIRE(iter != end);
  REQUIRE(*iter == 'H');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'e');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'l');
  ++iter;

  REQUIRE(iter != end);
  REQUIRE(*iter == 'l');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'o');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == ',');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == ' ');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'w');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'o');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'r');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'l');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'd');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == '!');
  ++iter;
  REQUIRE(iter == end);
  REQUIRE(iter.at_end());
  CHECK_THROWS_WITH(*iter, "Attempted to peek past the end of the string");
  CHECK_THROWS_WITH(++iter, "Attempted to advance past the end of the string");
}

TEST_CASE("can iterate over a String - Unicode") {
  String str("Hello, 🌍!");
  auto iter = str.begin();
  auto end = str.end();
  REQUIRE(iter != end);
  REQUIRE(*iter == 'H');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'e');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'l');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'l');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == 'o');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == ',');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == ' ');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == U'🌍');
  ++iter;
  REQUIRE(iter != end);
  REQUIRE(*iter == '!');
  ++iter;
  REQUIRE(iter == end);
  REQUIRE(iter.at_end());
  CHECK_THROWS_WITH(*iter, "Attempted to peek past the end of the string");
  CHECK_THROWS_WITH(++iter, "Attempted to advance past the end of the string");
}

TEST_CASE("empty String") {
  String str;
  size_t count = 0;
  for (auto _ : str) {
    ++count;
  }
  REQUIRE(count == 0);
}

TEST_SUITE_END();
