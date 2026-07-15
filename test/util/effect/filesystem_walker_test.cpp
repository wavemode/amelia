#include <doctest.h>

#include "util/effect/filesystem_walker.hpp"
#include "util/effect/console_printer.hpp"
#include "util/data/string.hpp"

TEST_SUITE_BEGIN("FilesystemWalker");

using namespace amelia;

TEST_CASE("all files") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "test/util/effect/fswalk", false);
  result.sort();

  REQUIRE(result[0] == "test/util/effect/fswalk/fswalk2");
  REQUIRE(result[1] == "test/util/effect/fswalk/fswalk2/test2.txt");
  REQUIRE(result[2] == "test/util/effect/fswalk/test.txt");
}

TEST_CASE("regular files only") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "test/util/effect/fswalk");
  result.sort();

  REQUIRE(result[0] == "test/util/effect/fswalk/fswalk2/test2.txt");
  REQUIRE(result[1] == "test/util/effect/fswalk/test.txt");
}

TEST_SUITE_END();
