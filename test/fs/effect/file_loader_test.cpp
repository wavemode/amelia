#include <doctest.h>

#include "fs/effect/file_loader.hpp"
#include "util/data/string.hpp"

TEST_SUITE_BEGIN("FileLoader");

using namespace amelia;

TEST_CASE("can read empty file") {
  FileLoader loader;
  String filename = "test/fs/effect/loader/empty.txt";
  String expected_contents = "";

  String result;
  loader.load_file(result, filename);
  REQUIRE(expected_contents == result);
}

TEST_CASE("can read ASCII") {
  FileLoader loader;
  String filename = "test/fs/effect/loader/hello.txt";
  String expected_contents = "Hello, world!\n";

  String result;
  loader.load_file(result, filename);
  REQUIRE(expected_contents == result);
}

TEST_CASE("can read UTF-8") {
  FileLoader loader;
  String filename = "test/fs/effect/loader/emojis.txt";
  String expected_contents = "✅⛔\n";

  String result;
  loader.load_file(result, filename);
  REQUIRE(expected_contents == result);
}

TEST_SUITE_END();
