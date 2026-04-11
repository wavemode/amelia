#include <cstdint>
#include <cstring>
#include <vendor/doctest.h>

#include "util/text/CharIterator.h"
#include "util/text/InvalidUTF8Error.h"
#include "util/text/String.h"

TEST_SUITE_BEGIN("CharIterator");

TEST_CASE("can iterate over a String - ASCII") {
  amelia::String str("Hello, world!");
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
}

TEST_CASE("can iterate over a String - Unicode") {
  amelia::String str("Hello, 🌍!");
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
}

TEST_CASE("empty String") {
  amelia::String str;
  size_t count = 0;
  for (auto _ : str) {
    ++count;
  }
  CHECK(count == 0);
}

TEST_SUITE_END();
