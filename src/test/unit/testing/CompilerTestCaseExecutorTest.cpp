#include "Prelude.h"
#include <doctest.h>

#include "effect/fs/FileLoader.h"
#include "effect/fs/FileWriter.h"
#include "effect/fs/FilesystemWalker.h"
#include "effect/sys/ConsolePrinter.h"
#include "effect/sys/EnvironmentReader.h"
#include "unit/testing/CompilerTestCaseCollector.h"
#include "unit/testing/CompilerTestCaseExecutor.h"
#include "unit/testing/LexerTestCaseRunner.h"

#include "data/text/TextUtils.h"

TEST_SUITE_BEGIN("CompilerTestCaseExecutor");

using namespace amelia;

TEST_CASE("execute") {
  FileLoader file_loader;
  FileWriter file_writer;
  ConsolePrinter console_printer;
  FilesystemWalker filesystem_walker;
  LexerTestCaseRunner lexer_test_case_runner;
  EnvironmentReader env_reader;
  CompilerTestCaseExecutor executor(
      &lexer_test_case_runner, &file_writer, &console_printer, &env_reader
  );

  String test_case_path = "src/test/unit/testing/execute/example1.am";
  String file_contents = "x = y\n";
  file_writer.write_file(test_case_path, file_contents);

  CompilerTestCaseCollection collection;
  CompilerTestCaseCollector collector(&filesystem_walker, &file_loader);
  collector.collect_test_cases(collection, "src/test/unit/testing/execute");
  REQUIRE(collection.test_cases.size() == 1);
  auto test_case = collection.test_cases[0];

  CHECK(executor.execute_test_case(test_case));
  CHECK(executor.update_expected_output(test_case));

  // running it again should return false, now that the expected output matches

  CompilerTestCaseCollection collection2;
  collector.collect_test_cases(collection2, "src/test/unit/testing/execute");
  REQUIRE(collection2.test_cases.size() == 1);
  auto test_case2 = collection2.test_cases[0];

  CHECK(!executor.execute_test_case(test_case2));
  CHECK(!executor.update_expected_output(test_case2));
}

TEST_SUITE_END();
