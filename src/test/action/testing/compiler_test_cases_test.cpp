#include "prelude.h"
#include <doctest.h>

#include "action/testing/compiler_test_cases.h"
#include "data/testing/compiler_test_case.h"
#include "data/testing/compiler_test_case_collection.h"
#include "effect/fs/file_loader.h"
#include "effect/fs/file_writer.h"
#include "effect/fs/filesystem_walker.h"
#include "effect/sys/environment_reader.h"
#include "effect/sys/silent_printer.h"
#include "unit/testing/lexer_test_case_runner.h"

TEST_SUITE_BEGIN("compiler_test_cases");

using namespace amelia;

TEST_CASE("collect_test_cases") {
  FileLoader file_loader;
  FilesystemWalker filesystem_walker;
  CompilerTestCaseCollection collection;

  collect_test_cases(filesystem_walker, file_loader, collection, "src/test/action/testing/collect");

  CHECK(collection.test_cases.size() == 2);
  CHECK(
      collection.test_cases[0].filename ==
      "src/test/action/testing/collect/collect1/collect2/test2.am"
  );
  CHECK(collection.test_cases[0].expected_output == "testing2\n");
  CHECK(collection.test_cases[1].filename == "src/test/action/testing/collect/collect1/test1.am");
  CHECK(collection.test_cases[1].expected_output == "testing1\n");
}

TEST_CASE("execute_test_case") {
  FileLoader file_loader;
  FileWriter file_writer;
  SilentPrinter console_printer;
  FilesystemWalker filesystem_walker;
  LexerTestCaseRunner lexer_test_case_runner;
  EnvironmentReader env_reader;

  String test_case_path = "src/test/action/testing/execute/example1.am";
  String file_contents = "x = y\n";
  file_writer.write_file(test_case_path, file_contents);

  CompilerTestCaseCollection collection;
  collect_test_cases(filesystem_walker, file_loader, collection, "src/test/action/testing/execute");
  REQUIRE(collection.test_cases.size() == 1);
  auto test_case = collection.test_cases[0];

  CHECK(execute_test_case(lexer_test_case_runner, test_case));
  CHECK(update_expected_output(lexer_test_case_runner, file_writer, test_case));

  // running it again should return false, now that the expected output matches

  CompilerTestCaseCollection collection2;
  collect_test_cases(
      filesystem_walker, file_loader, collection2, "src/test/action/testing/execute"
  );
  REQUIRE(collection2.test_cases.size() == 1);
  auto test_case2 = collection2.test_cases[0];

  CHECK(!execute_test_case(lexer_test_case_runner, test_case2));
  CHECK(!update_expected_output(lexer_test_case_runner, file_writer, test_case2));
}

TEST_SUITE_END();
