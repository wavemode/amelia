#include "Prelude.h"
#include <doctest.h>

#include <cstdint>
#include <cstring>
#include <stdexcept>

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
  CHECK_THROWS_AS(*iter, RuntimeError);
  CHECK_THROWS_AS(++iter, RuntimeError);
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
  CHECK_THROWS_AS(*iter, RuntimeError);
  CHECK_THROWS_AS(++iter, RuntimeError);
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
