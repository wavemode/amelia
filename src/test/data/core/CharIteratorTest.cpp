#include <doctest.h>

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "data/core/CharIterator.h"
#include "data/core/InvalidUTF8Error.h"
#include "data/core/String.h"

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
  CHECK_THROWS_AS(*iter, std::out_of_range);
  CHECK_THROWS_AS(++iter, std::out_of_range);
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
  CHECK_THROWS_AS(*iter, std::out_of_range);
  CHECK_THROWS_AS(++iter, std::out_of_range);
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
