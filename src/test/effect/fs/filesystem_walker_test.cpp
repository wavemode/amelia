#include <doctest.h>

#include "prelude.hpp"

#include "effect/fs/filesystem_walker.hpp"
#include "effect/sys/console_printer.hpp"

TEST_SUITE_BEGIN("FilesystemWalker");

using namespace amelia;

TEST_CASE("all files") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "src/test/effect/fs/fswalk", false);
  result.sort();

  REQUIRE(result[0] == "src/test/effect/fs/fswalk/fswalk2");
  REQUIRE(result[1] == "src/test/effect/fs/fswalk/fswalk2/test2.txt");
  REQUIRE(result[2] == "src/test/effect/fs/fswalk/test.txt");
}

TEST_CASE("regular files only") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "src/test/effect/fs/fswalk");
  result.sort();

  REQUIRE(result[0] == "src/test/effect/fs/fswalk/fswalk2/test2.txt");
  REQUIRE(result[1] == "src/test/effect/fs/fswalk/test.txt");
}

TEST_SUITE_END();
