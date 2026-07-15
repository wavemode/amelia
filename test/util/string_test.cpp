#include <cstring>

#include <doctest.h>

#include "util/data/string.hpp"
#include "util/data/char_iterator.hpp"
#include "util/data/invalid_utf8_error.hpp"

TEST_SUITE_BEGIN("String");

using namespace amelia;

TEST_CASE("default is empty") {
  String str;
  REQUIRE(str.size() == 0);
  REQUIRE(std::strcmp(str.c_str(), "") == 0);
}

TEST_CASE("can be constructed from a C-style string") {
  String str = "Hello, world!";
  REQUIRE(str.size() == 13);
  REQUIRE(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be constructed from an empty C-style string") {
  String str("");
  REQUIRE(str.size() == 0);
  REQUIRE(std::strcmp(str.c_str(), "") == 0);
}

TEST_CASE("can be constructed from another String") {
  String str1("Hello, world!");
  String str2(str1);
  REQUIRE(str2.size() == 13);
  REQUIRE(std::strcmp(str2.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be assigned from a C-style string") {
  String str;
  str = "Hello, world!";
  REQUIRE(str.size() == 13);
  REQUIRE(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be assigned from another String") {
  String str1("Hello, world!");
  String str2;
  str2 = str1;
  REQUIRE(str2.size() == 13);
  REQUIRE(std::strcmp(str2.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be constructed from a Slice of bytes") {
  const char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
  REQUIRE(sizeof(data) == 13);

  String str = String(ConstSlice(data));
  REQUIRE(str.size() == 13);
  REQUIRE(std::strcmp(str.c_str(), "Hello, world!") == 0);

  const char data2[] = "Hello, world!";
  REQUIRE(sizeof(data2) == 14);
  String str2 = String(ConstSlice(data2));

  // String will actually have a NULL character at the end, since the Slice does.
  REQUIRE(str2.size() == 14);
  REQUIRE(str2.c_str()[13] == '\0');
}

TEST_CASE("can be iterated over") {
  String str("Hello");
  auto iter = str.begin();
  auto end = str.end();
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
  REQUIRE(iter == end);
}

TEST_CASE("can be appended with a C-style string") {
  String str("Hello");
  str.append(", world!");
  REQUIRE(str.size() == 13);
  REQUIRE(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be appended with another String") {
  String str1("Hello");
  String str2(", world!");
  str1.append(str2);
  REQUIRE(str1.size() == 13);
  REQUIRE(std::strcmp(str1.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be appended with a UTF32 code point") {
  String str("Hello ");
  str.append(U'🌍');
  REQUIRE(str.size() == 10);
  REQUIRE(std::strcmp(str.c_str(), "Hello 🌍") == 0);
}

TEST_CASE("concatenation") {
  String str1("Hello, ");
  String str2("world!");
  String str3 = str1 + str2;
  REQUIRE(str3.size() == 13);
  REQUIRE(std::strcmp(str3.c_str(), "Hello, world!") == 0);
}

TEST_CASE("equality") {
  String str1("Hello, world!");
  String str2("Hello, world!");
  String str3("Goodbye, world!");
  REQUIRE(str1 == str2);
  REQUIRE(str1 != str3);
}

TEST_CASE("lexicographical comparison - ASCII") {
  String str1("Apple");
  String str2("Banana");
  String str3("Apple");
  String str4("Apple Tree");

  REQUIRE(str1 < str2);
  REQUIRE(str1 <= str2);
  REQUIRE(str1 == str3);
  REQUIRE(str1 >= str3);
  REQUIRE(str1 <= str3);
  REQUIRE(str1 < str4);
  REQUIRE(str1 <= str4);

  REQUIRE(str2 > str1);
  REQUIRE(str2 >= str1);
  REQUIRE(str2 > str3);
  REQUIRE(str2 >= str3);
  REQUIRE(str2 > str4);
  REQUIRE(str2 >= str4);

  REQUIRE(str3 == str1);
  REQUIRE(str3 >= str1);
  REQUIRE(str3 <= str1);
  REQUIRE(str3 < str2);
  REQUIRE(str3 <= str2);
  REQUIRE(str3 < str4);
  REQUIRE(str3 <= str4);

  REQUIRE(str4 > str1);
  REQUIRE(str4 >= str1);
  REQUIRE(str4 < str2);
  REQUIRE(str4 <= str2);
  REQUIRE(str4 > str3);
  REQUIRE(str4 >= str3);
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

  REQUIRE(str1 < str2);
  REQUIRE(str1 <= str2);
  REQUIRE(str1 > str3);
  REQUIRE(str1 >= str3);

  REQUIRE(str2 > str1);
  REQUIRE(str2 >= str1);
  REQUIRE(str2 > str3);
  REQUIRE(str2 >= str3);

  REQUIRE(str3 < str1);
  REQUIRE(str3 <= str1);
  REQUIRE(str3 < str2);
  REQUIRE(str3 <= str2);
}

TEST_CASE("invalid UTF-8") {
  CHECK_THROWS_AS(String("\xFF\xFF\xFF"), InvalidUTF8Error);
  String str;
  CHECK_THROWS_AS(str.append("\xFF\xFF\xFF"), InvalidUTF8Error);
}

TEST_SUITE_END();
