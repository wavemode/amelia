#include <cstring>
#include <vendor/doctest.h>

#include "util/text/InvalidUTF8Error.h"
#include "util/text/String.h"

TEST_SUITE_BEGIN("String");

TEST_CASE("default is empty") {
  amelia::String str;
  CHECK(str.size() == 0);
  CHECK(std::strcmp(str.c_str(), "") == 0);
}

TEST_CASE("can be constructed from a C-style string") {
  amelia::String str("Hello, world!");
  CHECK(str.size() == 13);
  CHECK(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be constructed from another String") {
  amelia::String str1("Hello, world!");
  amelia::String str2(str1);
  CHECK(str2.size() == 13);
  CHECK(std::strcmp(str2.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be assigned from a C-style string") {
  amelia::String str;
  str = "Hello, world!";
  CHECK(str.size() == 13);
  CHECK(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be assigned from another String") {
  amelia::String str1("Hello, world!");
  amelia::String str2;
  str2 = str1;
  CHECK(str2.size() == 13);
  CHECK(std::strcmp(str2.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be appended with a C-style string") {
  amelia::String str("Hello");
  str.append(", world!");
  CHECK(str.size() == 13);
  CHECK(std::strcmp(str.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be appended with another String") {
  amelia::String str1("Hello");
  amelia::String str2(", world!");
  str1.append(str2);
  CHECK(str1.size() == 13);
  CHECK(std::strcmp(str1.c_str(), "Hello, world!") == 0);
}

TEST_CASE("can be appended with a UTF32 code point") {
  amelia::String str("Hello ");
  str.append(U'🌍');
  CHECK(str.size() == 10);
  CHECK(std::strcmp(str.c_str(), "Hello 🌍") == 0);
}

TEST_CASE("concatenation") {
  amelia::String str1("Hello, ");
  amelia::String str2("world!");
  amelia::String str3 = str1 + str2;
  CHECK(str3.size() == 13);
  CHECK(std::strcmp(str3.c_str(), "Hello, world!") == 0);
}

TEST_CASE("equality") {
  amelia::String str1("Hello, world!");
  amelia::String str2("Hello, world!");
  amelia::String str3("Goodbye, world!");
  CHECK(str1 == str2);
  CHECK(str1 != str3);
}

TEST_CASE("lexicographical comparison - ASCII") {
  amelia::String str1("Apple");
  amelia::String str2("Banana");
  amelia::String str3("Apple");
  amelia::String str4("Apple Tree");

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
  amelia::String str1;
  uint32_t cp1 = 0x1F602;
  uint32_t cp2 = 0x1F603;
  str1.append(cp1);
  str1.append(cp2);
  REQUIRE(str1 == "😂😃");

  amelia::String str2;
  str2.append(cp2);
  REQUIRE(str2 == "😃");

  amelia::String str3;
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
  CHECK_THROWS_AS(amelia::String("\xFF\xFF\xFF"), amelia::InvalidUTF8Error);
  amelia::String str;
  CHECK_THROWS_AS(str.append("\xFF\xFF\xFF"), amelia::InvalidUTF8Error);
}

TEST_SUITE_END();
