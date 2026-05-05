#include <cstring>

#include <doctest.h>

#include "prelude.h"

TEST_SUITE_BEGIN("Text");

using namespace amelia;

TEST_CASE("can be constructed from a string literal") {
  Text text = "Hello, world!";
  CHECK(text.size() == 13);
  CHECK(std::memcmp(text.data().ptr(), "Hello, world!", 13) == 0);
}

TEST_CASE("can be constructed from an empty string literal") {
  Text text("");
  CHECK(text.size() == 0);
  CHECK(text.data().ptr() != nullptr);
}

TEST_CASE("default construction") {
  Text text;
  CHECK(text == Text(""));
}

TEST_CASE("can be constructed from an array of characters") {
  char arr[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
  Text text = arr;
  CHECK(text.size() == 13);
  CHECK(std::memcmp(text.data().ptr(), arr, 13) == 0);
}

TEST_CASE("can be constructed from a null-terminated array of characters") {
  char arr[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0'};
  Text text = arr;
  CHECK(text.size() == 13);
  CHECK(std::memcmp(text.data().ptr(), arr, 13) == 0);
}

TEST_CASE("can be constructed from a String") {
  String str("Hello, world!");
  Text text = str;
  CHECK(text.size() == 13);
  CHECK(std::memcmp(text.data().ptr(), "Hello, world!", 13) == 0);
}

TEST_CASE("can be constructed from a ConstSlice<char>") {
  const char arr[] = "Hello, world!";
  Text text(ConstSlice(arr, 13));
  CHECK(text.size() == 13);
  CHECK(std::memcmp(text.data().ptr(), arr, 13) == 0);
}

TEST_CASE("raises InvalidUTF8Error when constructed from invalid UTF-8 data") {
  CHECK_THROWS_AS(Text({char(0xC0), char(0xAF)}), InvalidUTF8Error);
}

TEST_CASE("raises InvalidUTF8Error when assigned invalid UTF-8 data") {
  const char invalid_utf8[] = {char(0xC0), char(0xAF)};
  Text text("");
  CHECK_THROWS_AS(text = invalid_utf8, InvalidUTF8Error);
}

TEST_CASE("can iterate over the characters in the text") {
  Text text("Hello, world!");
  const char expected[] = "Hello, world!";
  size_t i = 0;
  for (char c : text) {
    CHECK(c == expected[i]);
    ++i;
  }
  CHECK(i == text.size());
}

TEST_CASE("equality") {
  String str1("Hello, world!");
  String str2("Hello, world!");
  String str3("Goodbye, world!");
  Text text1(str1);
  Text text2(str2);
  Text text3(str3);

  CHECK(text1 == text2);
  CHECK(text1 != text3);
  CHECK(text2 != text3);
}

TEST_CASE("lexicographical comparison - ASCII") {
  String str1("Apple");
  String str2("Banana");
  String str3("Apple");
  String str4("Apple Tree");
  Text text1(str1);
  Text text2(str2);
  Text text3(str3);
  Text text4(str4);

  CHECK(text1 < text2);
  CHECK(text1 <= text2);
  CHECK(text1 == text3);
  CHECK(text1 >= text3);
  CHECK(text1 <= text3);
  CHECK(text1 < text4);
  CHECK(text1 <= text4);

  CHECK(text2 > text1);
  CHECK(text2 >= text1);
  CHECK(text2 > text3);
  CHECK(text2 >= text3);
  CHECK(text2 > text4);
  CHECK(text2 >= text4);

  CHECK(text3 == text1);
  CHECK(text3 >= text1);
  CHECK(text3 <= text1);
  CHECK(text3 < text2);
  CHECK(text3 <= text2);
  CHECK(text3 < text4);
  CHECK(text3 <= text4);

  CHECK(text4 > text1);
  CHECK(text4 >= text1);
  CHECK(text4 < text2);
  CHECK(text4 <= text2);
  CHECK(text4 > text3);
  CHECK(text4 >= text3);
}

TEST_CASE("lexicographical comparison - Unicode") {
  uint32_t cp1 = 0x1F602;
  uint32_t cp2 = 0x1F603;

  String str1;
  str1.append(cp1);
  str1.append(cp2);
  REQUIRE(str1 == "😂😃");

  String str2;
  str2.append(cp2);
  REQUIRE(str2 == "😃");

  String str3;
  str3.append(cp1);
  REQUIRE(str3 == "😂");

  Text text1(str1);
  Text text2(str2);
  Text text3(str3);

  CHECK(text1 < text2);
  CHECK(text1 <= text2);
  CHECK(text1 > text3);
  CHECK(text1 >= text3);

  CHECK(text2 > text1);
  CHECK(text2 >= text1);
  CHECK(text2 > text3);
  CHECK(text2 >= text3);

  CHECK(text3 < text1);
  CHECK(text3 <= text1);
  CHECK(text3 < text2);
  CHECK(text3 <= text2);
}

TEST_SUITE_END();
