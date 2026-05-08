#include <cstring>

#include <doctest.h>

#include "prelude.h"

TEST_SUITE_BEGIN("Slice");

using namespace amelia;

TEST_CASE("can iterate over elements") {
  ConstSlice<char> slice("Hello");

  // for-each
  size_t count = 0;
  for (auto ch : slice) {
    CHECK(ch == "Hello"[count++]);
  }

  // manual
  auto iter = slice.begin();
  auto end = slice.end();
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
  CHECK(*iter == '\0');
  ++iter;
  CHECK(iter == end);
  CHECK(iter.size() == 0);
  CHECK_THROWS_WITH(*iter, "Dereferencing end of slice");
  CHECK_THROWS_WITH(++iter, "Slice iterator offset out of range");
}

TEST_CASE("can construct from array") {
  ConstSlice<char> slice("Hello, world!");
  CHECK(slice.size() == 14); // includes null terminator
  CHECK(String(slice) == "Hello, world!\0");

  char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
  ConstSlice<char> slice2(data);
  CHECK(slice2.size() == 13);
  CHECK(String(slice2) == "Hello, world!");
}

TEST_CASE("can construct from pointer and length") {
  const char data[] = "Hello, world!";
  REQUIRE(sizeof(data) == 14); // includes null terminator
  ConstSlice<char> slice(data, 13);
  CHECK(slice.size() == 13);
  CHECK(String(slice) == "Hello, world!");
}

TEST_CASE("can index") {
  ConstSlice<char> slice("Hello");
  CHECK(slice[0] == 'H');
  CHECK(slice[1] == 'e');
  CHECK(slice[2] == 'l');
  CHECK(slice[3] == 'l');
  CHECK(slice[4] == 'o');
  CHECK(slice[5] == '\0');
  CHECK_THROWS_WITH(slice[6], "Slice index out of range");
}

TEST_CASE("equality and inequality") {
  ConstSlice<char> slice1("Hello");
  ConstSlice<char> slice2("Hello");
  ConstSlice<char> slice3("World");

  CHECK(slice1 == slice2);
  CHECK(slice1 != slice3);
  CHECK(slice2 != slice3);
}

TEST_SUITE_END();
