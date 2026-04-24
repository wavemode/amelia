#include <doctest.h>

#include <algorithm>

#include "Prelude.h"
#include "data/core/ListUtils.h"

#include "effect/fs/FilesystemWalker.h"
#include "effect/sys/ConsolePrinter.h"

TEST_SUITE_BEGIN("FilesystemWalker");

using namespace amelia;

TEST_CASE("all files") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "src/test/effect/fs/fswalk", false);
  ListUtils::sort(result);

  CHECK(result[0] == "src/test/effect/fs/fswalk/fswalk2");
  CHECK(result[1] == "src/test/effect/fs/fswalk/fswalk2/test2.txt");
  CHECK(result[2] == "src/test/effect/fs/fswalk/test.txt");
}

TEST_CASE("regular files only") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk(result, "src/test/effect/fs/fswalk");
  ListUtils::sort(result);

  CHECK(result[0] == "src/test/effect/fs/fswalk/fswalk2/test2.txt");
  CHECK(result[1] == "src/test/effect/fs/fswalk/test.txt");
}

TEST_SUITE_END();
