#include <doctest.h>

#include "effect/core/FileLoader.h"
#include "util/text/String.h"

TEST_SUITE_BEGIN("FileLoader");

using namespace amelia;

TEST_CASE("can read empty file") {
  FileLoader loader;
  String filename = "src/test/effect/core/empty.txt";
  String expected_contents = "";

  String result = loader.load_file(filename);
  CHECK(expected_contents == result);
}

TEST_CASE("can read ASCII") {
  FileLoader loader;
  String filename = "src/test/effect/core/hello.txt";
  String expected_contents = "Hello, world!\n";

  String result = loader.load_file(filename);
  CHECK(expected_contents == result);
}

TEST_CASE("can read UTF-8") {
  FileLoader loader;
  String filename = "src/test/effect/core/emojis.txt";
  String expected_contents = "✅⛔\n";

  String result = loader.load_file(filename);
  CHECK(expected_contents == result);
}

TEST_SUITE_END();
