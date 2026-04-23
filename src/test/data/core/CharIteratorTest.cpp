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

TEST_CASE("find substring - ASCII") {
  String str("Hello, world!");
  CharIterator iter = str.begin();
  CharIterator position = iter.find("world");
  CHECK(iter.tail(position) == "world!");
}

TEST_CASE("find substring - Unicode") {
  String str("Hello, 🌍!");
  CharIterator iter = str.begin();
  CharIterator result = iter.find("🌍");
  CHECK(iter.tail(result) == "🌍!");
}

TEST_CASE("fail to find substring") {
  String str("Hello, world!");
  CharIterator iter = str.begin();
  CharIterator result = iter.find("foo");
  CHECK(result.at_end());
}

TEST_CASE("find code point - ASCII") {
  String str("Hello, world!");
  CharIterator iter = str.begin();
  CharIterator result = iter.find('w');
  CHECK(iter.tail(result) == "world!");
}

TEST_CASE("find code point - Unicode") {
  String str("Hello, 🌍!");
  CharIterator iter = str.begin();
  CharIterator result = iter.find(U'🌍');
  CHECK(iter.tail(result) == "🌍!");
}

TEST_CASE("fail to find code point") {
  String str("Hello, world!");
  CharIterator iter = str.begin();
  CharIterator result = iter.find('x');
  CHECK(result.at_end());
}

TEST_CASE("head and tail") {
  String str("Hello, world!");
  CharIterator iter = str.begin();
  CharIterator comma_pos = iter.find(',');
  CHECK(iter.head(comma_pos) == "Hello");
  CHECK(iter.tail(comma_pos) == ", world!");
}

TEST_CASE("subslice") {
  String str("Hello, world!");
  CharIterator iter = str.begin();
  CharIterator comma_pos = iter.find(',');
  CharIterator iter_end = iter.end();
  CHECK(iter.subslice(iter, comma_pos) == "Hello");
  CHECK(iter.subslice(comma_pos, iter_end) == ", world!");

  // subslice with end before start will result in a slice length wraparound
  CHECK_THROWS_AS(iter.subslice(comma_pos, iter), std::out_of_range);
}

TEST_SUITE_END();
