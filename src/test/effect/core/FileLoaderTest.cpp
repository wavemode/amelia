#include <doctest.h>

#include "effect/core/FileLoader.h"
#include "util/text/String.h"
#include "util/text/Text.h"

TEST_SUITE_BEGIN("FileLoader");

TEST_CASE("can read empty file") {
  amelia::FileLoader loader;
  amelia::String filename = "src/test/effect/core/empty.txt";
  amelia::String expected_contents = "";

  amelia::String result = loader.load_file(amelia::Text(filename));
  CHECK(expected_contents == result);
}

TEST_CASE("can read ASCII") {
  amelia::FileLoader loader;
  amelia::String filename = "src/test/effect/core/hello.txt";
  amelia::String expected_contents = "Hello, world!\n";

  amelia::String result = loader.load_file(amelia::Text(filename));
  CHECK(expected_contents == result);
}

TEST_CASE("can read UTF-8") {
  amelia::FileLoader loader;
  amelia::String filename = "src/test/effect/core/emojis.txt";
  amelia::String expected_contents = "✅⛔\n";

  amelia::String result = loader.load_file(amelia::Text(filename));
  CHECK(expected_contents == result);
}

TEST_SUITE_END();
