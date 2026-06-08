#include <cstring>

#include <doctest.h>

#include "prelude.hpp"

TEST_SUITE_BEGIN("Slice");

using namespace amelia;

TEST_CASE("can iterate over elements") {
  ConstSlice<char> slice("Hello");

  // for-each
  size_t count = 0;
  for (auto ch : slice) {
    REQUIRE(ch == "Hello"[count++]);
  }

  // manual
  auto iter = slice.begin();
  auto end = slice.end();
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
  REQUIRE(*iter == '\0');
  ++iter;
  REQUIRE(iter == end);
  REQUIRE(iter.size() == 0);
  CHECK_THROWS_WITH(*iter, "Dereferencing end of slice");
  CHECK_THROWS_WITH(++iter, "Slice iterator offset out of range");
}

TEST_CASE("can construct from array") {
  ConstSlice<char> slice("Hello, world!");
  REQUIRE(slice.size() == 14); // includes null terminator
  REQUIRE(String(slice) == "Hello, world!\0");

  char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
  ConstSlice<char> slice2(data);
  REQUIRE(slice2.size() == 13);
  REQUIRE(String(slice2) == "Hello, world!");
}

TEST_CASE("can construct from pointer and length") {
  const char data[] = "Hello, world!";
  REQUIRE(sizeof(data) == 14); // includes null terminator
  ConstSlice<char> slice(data, 13);
  REQUIRE(slice.size() == 13);
  REQUIRE(String(slice) == "Hello, world!");
}

TEST_CASE("can index") {
  ConstSlice<char> slice("Hello");
  REQUIRE(slice[0] == 'H');
  REQUIRE(slice[1] == 'e');
  REQUIRE(slice[2] == 'l');
  REQUIRE(slice[3] == 'l');
  REQUIRE(slice[4] == 'o');
  REQUIRE(slice[5] == '\0');
  CHECK_THROWS_WITH(slice[6], "Slice index out of range");
}

TEST_CASE("equality and inequality") {
  ConstSlice<char> slice1("Hello");
  ConstSlice<char> slice2("Hello");
  ConstSlice<char> slice3("World");

  REQUIRE(slice1 == slice2);
  REQUIRE(slice1 != slice3);
  REQUIRE(slice2 != slice3);
}

TEST_SUITE_END();
