#include <cstring>
#include <vendor/doctest.h>

#include "util/slice/Slice.h"
#include "util/text/String.h"

TEST_SUITE_BEGIN("Slice");

TEST_CASE("can iterate over elements") {
  amelia::Slice<const char> slice("Hello");

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
  CHECK(iter.at_end());
  CHECK_THROWS_AS(*iter, std::out_of_range);
  CHECK_THROWS_AS(++iter, std::out_of_range);
}

TEST_CASE("can construct from array") {
  amelia::Slice<const char> slice("Hello, world!");
  CHECK(slice.size() == 14); // includes null terminator
  CHECK(amelia::String(slice) == "Hello, world!\0");

  amelia::Slice<const char> slice2(
      {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'});
  CHECK(slice2.size() == 13);
  CHECK(amelia::String(slice2) == "Hello, world!");
}

TEST_CASE("can construct from pointer and length") {
  const char data[] = "Hello, world!";
  REQUIRE(sizeof(data) == 14); // includes null terminator
  amelia::Slice<const char> slice(data, 13);
  CHECK(slice.size() == 13);
  CHECK(amelia::String(slice) == "Hello, world!");
}

TEST_CASE("can index") {
  amelia::Slice<const char> slice("Hello");
  CHECK(slice[0] == 'H');
  CHECK(slice[1] == 'e');
  CHECK(slice[2] == 'l');
  CHECK(slice[3] == 'l');
  CHECK(slice[4] == 'o');
  CHECK(slice[5] == '\0');
  CHECK_THROWS_AS(slice[6], std::out_of_range);
}

TEST_CASE("can increment and add") {
  amelia::Slice<const char> slice({'H', 'e', 'l', 'l', 'o'});
  slice += 2;
  CHECK(slice.size() == 3);
  CHECK(amelia::String(slice) == "llo");

  auto tmp = ++slice;
  CHECK(slice.size() == 2);
  CHECK(amelia::String(slice) == "lo");
  CHECK(tmp.size() == 2);
  CHECK(amelia::String(tmp) == "lo");

  slice = slice + 1;
  CHECK(slice.size() == 1);
  CHECK(amelia::String(slice) == "o");

  auto tmp2 = slice++;
  CHECK(slice.size() == 0);
  CHECK(amelia::String(slice) == "");
  CHECK(tmp2.size() == 1);
  CHECK(amelia::String(tmp2) == "o");

  CHECK_THROWS_AS(++slice, std::out_of_range);
}

TEST_CASE("equality and inequality") {
  amelia::Slice<const char> slice1("Hello");
  amelia::Slice<const char> slice2("Hello");
  amelia::Slice<const char> slice3("World");

  CHECK(slice1 == slice2);
  CHECK(slice1 != slice3);
  CHECK(slice2 != slice3);
}

TEST_SUITE_END();
