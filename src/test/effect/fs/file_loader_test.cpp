#include <doctest.h>

#include "prelude.h"

#include "effect/fs/file_loader.h"

TEST_SUITE_BEGIN("FileLoader");

using namespace amelia;

TEST_CASE("can read empty file") {
  FileLoader loader;
  String filename = "src/test/effect/fs/loader/empty.txt";
  String expected_contents = "";

  String result;
  loader.load_file(result, filename);
  CHECK(expected_contents == result);
}

TEST_CASE("can read ASCII") {
  FileLoader loader;
  String filename = "src/test/effect/fs/loader/hello.txt";
  String expected_contents = "Hello, world!\n";

  String result;
  loader.load_file(result, filename);
  CHECK(expected_contents == result);
}

TEST_CASE("can read UTF-8") {
  FileLoader loader;
  String filename = "src/test/effect/fs/loader/emojis.txt";
  String expected_contents = "✅⛔\n";

  String result;
  loader.load_file(result, filename);
  CHECK(expected_contents == result);
}

TEST_SUITE_END();
