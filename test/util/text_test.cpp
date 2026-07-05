#include <cstring>

#include <doctest.h>

#include "prelude.hpp"

TEST_SUITE_BEGIN("Text");

using namespace amelia;

TEST_CASE("can be constructed from a string literal") {
  Text text = "Hello, world!";
  REQUIRE(text.size() == 13);
  REQUIRE(std::memcmp(text.data().ptr(), "Hello, world!", 13) == 0);
}

TEST_CASE("can be constructed from an empty string literal") {
  Text text("");
  REQUIRE(text.size() == 0);
  REQUIRE(text.data().ptr() != nullptr);
}

TEST_CASE("default construction") {
  Text text;
  REQUIRE(text == Text(""));
}

TEST_CASE("can be constructed from an array of characters") {
  char arr[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
  Text text = arr;
  REQUIRE(text.size() == 13);
  REQUIRE(std::memcmp(text.data().ptr(), arr, 13) == 0);
}

TEST_CASE("can be constructed from a null-terminated array of characters") {
  char arr[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0'};
  Text text = arr;
  REQUIRE(text.size() == 13);
  REQUIRE(std::memcmp(text.data().ptr(), arr, 13) == 0);
}

TEST_CASE("can be constructed from a String") {
  String str("Hello, world!");
  Text text = str;
  REQUIRE(text.size() == 13);
  REQUIRE(std::memcmp(text.data().ptr(), "Hello, world!", 13) == 0);
}

TEST_CASE("can be constructed from a ConstSlice<char>") {
  const char arr[] = "Hello, world!";
  Text text(ConstSlice(arr, 13));
  REQUIRE(text.size() == 13);
  REQUIRE(std::memcmp(text.data().ptr(), arr, 13) == 0);
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
    REQUIRE(c == expected[i]);
    ++i;
  }
  REQUIRE(i == text.size());
}

TEST_CASE("equality") {
  String str1("Hello, world!");
  String str2("Hello, world!");
  String str3("Goodbye, world!");
  Text text1(str1);
  Text text2(str2);
  Text text3(str3);

  REQUIRE(text1 == text2);
  REQUIRE(text1 != text3);
  REQUIRE(text2 != text3);
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

  REQUIRE(text1 < text2);
  REQUIRE(text1 <= text2);
  REQUIRE(text1 == text3);
  REQUIRE(text1 >= text3);
  REQUIRE(text1 <= text3);
  REQUIRE(text1 < text4);
  REQUIRE(text1 <= text4);

  REQUIRE(text2 > text1);
  REQUIRE(text2 >= text1);
  REQUIRE(text2 > text3);
  REQUIRE(text2 >= text3);
  REQUIRE(text2 > text4);
  REQUIRE(text2 >= text4);

  REQUIRE(text3 == text1);
  REQUIRE(text3 >= text1);
  REQUIRE(text3 <= text1);
  REQUIRE(text3 < text2);
  REQUIRE(text3 <= text2);
  REQUIRE(text3 < text4);
  REQUIRE(text3 <= text4);

  REQUIRE(text4 > text1);
  REQUIRE(text4 >= text1);
  REQUIRE(text4 < text2);
  REQUIRE(text4 <= text2);
  REQUIRE(text4 > text3);
  REQUIRE(text4 >= text3);
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

  REQUIRE(text1 < text2);
  REQUIRE(text1 <= text2);
  REQUIRE(text1 > text3);
  REQUIRE(text1 >= text3);

  REQUIRE(text2 > text1);
  REQUIRE(text2 >= text1);
  REQUIRE(text2 > text3);
  REQUIRE(text2 >= text3);

  REQUIRE(text3 < text1);
  REQUIRE(text3 <= text1);
  REQUIRE(text3 < text2);
  REQUIRE(text3 <= text2);
}

TEST_SUITE_END();
