#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <doctest.h>

#include "prelude.hpp"

TEST_SUITE_BEGIN("CharIterator");

using namespace amelia;

TEST_CASE("can iterate over a String - ASCII") {
  String str("Hello, world!");
  auto iter = str.begin();
  auto end = str.end();
  CHECK(iter != end);
  CHECK(*iter == 'H');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'e');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'l');
  ++iter;

  CHECK(iter != end);
  CHECK(*iter == 'l');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'o');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == ',');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == ' ');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'w');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'o');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'r');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'l');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'd');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == '!');
  ++iter;
  CHECK(iter == end);
  CHECK(iter.at_end());
  CHECK_THROWS_WITH(*iter, "Attempted to peek past the end of the string");
  CHECK_THROWS_WITH(++iter, "Attempted to advance past the end of the string");
}

TEST_CASE("can iterate over a String - Unicode") {
  String str("Hello, 🌍!");
  auto iter = str.begin();
  auto end = str.end();
  CHECK(iter != end);
  CHECK(*iter == 'H');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'e');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'l');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'l');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == 'o');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == ',');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == ' ');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == U'🌍');
  ++iter;
  CHECK(iter != end);
  CHECK(*iter == '!');
  ++iter;
  CHECK(iter == end);
  CHECK(iter.at_end());
  CHECK_THROWS_WITH(*iter, "Attempted to peek past the end of the string");
  CHECK_THROWS_WITH(++iter, "Attempted to advance past the end of the string");
}

TEST_CASE("empty String") {
  String str;
  size_t count = 0;
  for (auto _ : str) {
    ++count;
  }
  CHECK(count == 0);
}

TEST_SUITE_END();
