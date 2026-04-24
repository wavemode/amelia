#include "Prelude.h"
#include <doctest.h>

#include "data/testing/CompilerTestCase.h"
#include "data/testing/CompilerTestCaseCollection.h"
#include "effect/fs/FileLoader.h"
#include "effect/fs/FilesystemWalker.h"
#include "effect/sys/ConsolePrinter.h"
#include "unit/testing/CompilerTestCaseCollector.h"

#include "data/core/ListUtils.h"
#include "data/text/TextUtils.h"

TEST_SUITE_BEGIN("CompilerTestCaseCollector");

using namespace amelia;

TEST_CASE("collect") {
  FileLoader file_loader;
  FilesystemWalker filesystem_walker;
  CompilerTestCaseCollector collector(&filesystem_walker, &file_loader);
  CompilerTestCaseCollection collection;

  collector.collect_test_cases(collection, "src/test/unit/testing/collect");

  CHECK(collection.test_cases.size() == 2);
  CHECK(
      collection.test_cases[0].filename ==
      "src/test/unit/testing/collect/collect1/collect2/test2.am"
  );
  CHECK(collection.test_cases[0].expected_output == "testing2\n");
  CHECK(collection.test_cases[1].filename == "src/test/unit/testing/collect/collect1/test1.am");
  CHECK(collection.test_cases[1].expected_output == "testing1\n");
}

TEST_SUITE_END();
