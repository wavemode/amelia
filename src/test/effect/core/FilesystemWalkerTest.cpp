#include <doctest.h>

#include <algorithm>

#include "data/core/ListUtils.h"
#include "data/core/String.h"
#include "effect/core/ConsolePrinter.h"
#include "effect/core/FilesystemWalker.h"

TEST_SUITE_BEGIN("FilesystemWalker");

using namespace amelia;

TEST_CASE("all files") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk("src/test/effect/core/fswalk", result, false);
  ListUtils::sort(result);

  CHECK(result[0] == "src/test/effect/core/fswalk/fswalk2");
  CHECK(result[1] == "src/test/effect/core/fswalk/fswalk2/test2.txt");
  CHECK(result[2] == "src/test/effect/core/fswalk/test.txt");
}

TEST_CASE("regular files only") {
  FilesystemWalker fsw;
  List<String> result;
  fsw.walk("src/test/effect/core/fswalk", result);
  ListUtils::sort(result);

  CHECK(result[0] == "src/test/effect/core/fswalk/fswalk2/test2.txt");
  CHECK(result[1] == "src/test/effect/core/fswalk/test.txt");
}

TEST_SUITE_END();
