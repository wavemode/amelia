#include <doctest.h>

#include <algorithm>
#include <vector>

#include "effect/core/ConsolePrinter.h"
#include "effect/core/FilesystemWalker.h"
#include "util/text/String.h"

TEST_SUITE_BEGIN("FilesystemWalker");

using namespace amelia;

TEST_CASE("all files") {
  FilesystemWalker fsw;
  std::vector<String> result;
  fsw.walk("src/test/effect/core/fswalk", result, false);
  std::sort(result.begin(), result.end());

  CHECK(result[0] == "src/test/effect/core/fswalk/fswalk2");
  CHECK(result[1] == "src/test/effect/core/fswalk/fswalk2/test2.txt");
  CHECK(result[2] == "src/test/effect/core/fswalk/test.txt");
}

TEST_CASE("regular files only") {
  FilesystemWalker fsw;
  std::vector<String> result;
  fsw.walk("src/test/effect/core/fswalk", result);
  std::sort(result.begin(), result.end());

  CHECK(result[0] == "src/test/effect/core/fswalk/fswalk2/test2.txt");
  CHECK(result[1] == "src/test/effect/core/fswalk/test.txt");
}

TEST_SUITE_END();
