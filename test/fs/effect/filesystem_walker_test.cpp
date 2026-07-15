#include <doctest.h>

#include "fs/effect/filesystem_walker.hpp"
#include "sys/effect/console_printer.hpp"
#include "util/data/string.hpp"

TEST_SUITE_BEGIN("FilesystemWalker");

using namespace amelia;

TEST_CASE("all files") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "test/fs/effect/fswalk", false);
  result.sort();

  REQUIRE(result[0] == "test/fs/effect/fswalk/fswalk2");
  REQUIRE(result[1] == "test/fs/effect/fswalk/fswalk2/test2.txt");
  REQUIRE(result[2] == "test/fs/effect/fswalk/test.txt");
}

TEST_CASE("regular files only") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "test/fs/effect/fswalk");
  result.sort();

  REQUIRE(result[0] == "test/fs/effect/fswalk/fswalk2/test2.txt");
  REQUIRE(result[1] == "test/fs/effect/fswalk/test.txt");
}

TEST_SUITE_END();
