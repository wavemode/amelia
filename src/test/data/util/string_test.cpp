#include <cstring>

#include <doctest.h>

#include "prelude.h"

TEST_SUITE_BEGIN("String");

using namespace amelia;

TEST_CASE("default is empty") {
  String str;
  CHECK(str.size() == 0);
  CHECK(std::strcmp(str.c_str(), "") == 0);
}

TEST_CASE("can be constructed from a C-style string") {
  String str = "Hello, world!";
  CHECK(str.size() == 13);
  CHECK(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be constructed from an empty C-style string") {
  String str("");
  CHECK(str.size() == 0);
  CHECK(std::strcmp(str.c_str(), "") == 0);
}

TEST_CASE("can be constructed from another String") {
  String str1("Hello, world!");
  String str2(str1);
  CHECK(str2.size() == 13);
  CHECK(std::strcmp(str2.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be assigned from a C-style string") {
  String str;
  str = "Hello, world!";
  CHECK(str.size() == 13);
  CHECK(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be assigned from another String") {
  String str1("Hello, world!");
  String str2;
  str2 = str1;
  CHECK(str2.size() == 13);
  CHECK(std::strcmp(str2.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be constructed from a Slice of bytes") {
  const char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
  REQUIRE(sizeof(data) == 13);

  String str = String(ConstSlice(data));
  CHECK(str.size() == 13);
  CHECK(std::strcmp(str.c_str(), "Hello, world!") == 0);

  const char data2[] = "Hello, world!";
  REQUIRE(sizeof(data2) == 14);
  String str2 = String(ConstSlice(data2));

  // String will actually have a NULL character at the end, since the Slice does.
  CHECK(str2.size() == 14);
  CHECK(str2.c_str()[13] == '\0');
}

TEST_CASE("can be iterated over") {
  String str("Hello");
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
  CHECK(iter == end);
}

TEST_CASE("can be appended with a C-style string") {
  String str("Hello");
  str.append(", world!");
  CHECK(str.size() == 13);
  CHECK(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be appended with another String") {
  String str1("Hello");
  String str2(", world!");
  str1.append(str2);
  CHECK(str1.size() == 13);
  CHECK(std::strcmp(str1.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be appended with a UTF32 code point") {
  String str("Hello ");
  str.append(U'🌍');
  CHECK(str.size() == 10);
  CHECK(std::strcmp(str.c_str(), "Hello 🌍") == 0);
}

TEST_CASE("concatenation") {
  String str1("Hello, ");
  String str2("world!");
  String str3 = str1 + str2;
  CHECK(str3.size() == 13);
  CHECK(std::strcmp(str3.c_str(), "Hello, world!") == 0);
}

TEST_CASE("equality") {
  String str1("Hello, world!");
  String str2("Hello, world!");
  String str3("Goodbye, world!");
  CHECK(str1 == str2);
  CHECK(str1 != str3);
}

TEST_CASE("lexicographical comparison - ASCII") {
  String str1("Apple");
  String str2("Banana");
  String str3("Apple");
  String str4("Apple Tree");

  CHECK(str1 < str2);
  CHECK(str1 <= str2);
  CHECK(str1 == str3);
  CHECK(str1 >= str3);
  CHECK(str1 <= str3);
  CHECK(str1 < str4);
  CHECK(str1 <= str4);

  CHECK(str2 > str1);
  CHECK(str2 >= str1);
  CHECK(str2 > str3);
  CHECK(str2 >= str3);
  CHECK(str2 > str4);
  CHECK(str2 >= str4);

  CHECK(str3 == str1);
  CHECK(str3 >= str1);
  CHECK(str3 <= str1);
  CHECK(str3 < str2);
  CHECK(str3 <= str2);
  CHECK(str3 < str4);
  CHECK(str3 <= str4);

  CHECK(str4 > str1);
  CHECK(str4 >= str1);
  CHECK(str4 < str2);
  CHECK(str4 <= str2);
  CHECK(str4 > str3);
  CHECK(str4 >= str3);
}

TEST_CASE("lexicographical comparison - Unicode") {
  String str1;
  uint32_t cp1 = 0x1F602;
  uint32_t cp2 = 0x1F603;
  str1.append(cp1);
  str1.append(cp2);
  REQUIRE(str1 == "😂😃");

  String str2;
  str2.append(cp2);
  REQUIRE(str2 == "😃");

  String str3;
  str3.append(cp1);
  REQUIRE(str3 == "😂");

  CHECK(str1 < str2);
  CHECK(str1 <= str2);
  CHECK(str1 > str3);
  CHECK(str1 >= str3);

  CHECK(str2 > str1);
  CHECK(str2 >= str1);
  CHECK(str2 > str3);
  CHECK(str2 >= str3);

  CHECK(str3 < str1);
  CHECK(str3 <= str1);
  CHECK(str3 < str2);
  CHECK(str3 <= str2);
}

TEST_CASE("invalid UTF-8") {
  CHECK_THROWS_AS(String("\xFF\xFF\xFF"), InvalidUTF8Error);
  String str;
  CHECK_THROWS_AS(str.append("\xFF\xFF\xFF"), InvalidUTF8Error);
}

TEST_SUITE_END();
