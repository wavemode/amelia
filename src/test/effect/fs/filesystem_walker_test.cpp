#include <doctest.h>

#include "prelude.h"

#include "effect/fs/filesystem_walker.h"
#include "effect/sys/console_printer.h"

TEST_SUITE_BEGIN("FilesystemWalker");

using namespace amelia;

TEST_CASE("all files") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "src/test/effect/fs/fswalk", false);
  result.sort();

  CHECK(result[0] == "src/test/effect/fs/fswalk/fswalk2");
  CHECK(result[1] == "src/test/effect/fs/fswalk/fswalk2/test2.txt");
  CHECK(result[2] == "src/test/effect/fs/fswalk/test.txt");
}

TEST_CASE("regular files only") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "src/test/effect/fs/fswalk");
  result.sort();

  CHECK(result[0] == "src/test/effect/fs/fswalk/fswalk2/test2.txt");
  CHECK(result[1] == "src/test/effect/fs/fswalk/test.txt");
}

TEST_SUITE_END();
