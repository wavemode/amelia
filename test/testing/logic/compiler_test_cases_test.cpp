#include <doctest.h>

#include "testing/data/compiler_test_case.hpp"
#include "testing/data/compiler_test_case_collection.hpp"
#include "testing/logic/compiler_test_cases.hpp"
#include "testing/system/lexer_test_case_runner.hpp"
#include "util/effect/environment_reader.hpp"
#include "util/effect/file_loader.hpp"
#include "util/effect/file_writer.hpp"
#include "util/effect/filesystem_walker.hpp"
#include "util/effect/silent_printer.hpp"

TEST_SUITE_BEGIN("compiler_test_cases");

using namespace amelia;

TEST_CASE("collect_test_cases") {
  FileLoader file_loader;
  FilesystemWalker filesystem_walker;
  EnvironmentReader env_reader;
  SilentPrinter silent_printer;
  CompilerTestCaseCollection collection;

  collect_test_cases(
      filesystem_walker,
      file_loader,
      silent_printer,
      env_reader,
      collection,
      "test/testing/logic/collect"
  );

  REQUIRE(collection.test_cases.size() == 2);
  REQUIRE(
      collection.test_cases[0].filename == "test/testing/logic/collect/collect1/collect2/test2.am"
  );
  REQUIRE(collection.test_cases[0].expected_output == "testing2\n");
  REQUIRE(collection.test_cases[1].filename == "test/testing/logic/collect/collect1/test1.am");
  REQUIRE(collection.test_cases[1].expected_output == "testing1\n");
}

TEST_CASE("execute_test_case") {
  FileLoader file_loader;
  FileWriter file_writer;
  SilentPrinter silent_printer;
  FilesystemWalker filesystem_walker;
  LexerTestCaseRunner lexer_test_case_runner;
  EnvironmentReader env_reader;

  String test_case_path = "test/testing/logic/execute/example1.am";
  String file_contents = "x = y\n";
  file_writer.write_file(test_case_path, file_contents);

  CompilerTestCaseCollection collection;
  collect_test_cases(
      filesystem_walker,
      file_loader,
      silent_printer,
      env_reader,
      collection,
      "test/testing/logic/execute"
  );
  REQUIRE(collection.test_cases.size() == 1);
  auto test_case = collection.test_cases[0];

  REQUIRE(execute_test_case(lexer_test_case_runner, test_case));
  REQUIRE(update_expected_output(lexer_test_case_runner, file_writer, test_case));

  // running it again should return false, now that the expected output matches

  CompilerTestCaseCollection collection2;
  collect_test_cases(
      filesystem_walker,
      file_loader,
      silent_printer,
      env_reader,
      collection2,
      "test/testing/logic/execute"
  );
  REQUIRE(collection2.test_cases.size() == 1);
  auto test_case2 = collection2.test_cases[0];

  REQUIRE(!execute_test_case(lexer_test_case_runner, test_case2));
  REQUIRE(!update_expected_output(lexer_test_case_runner, file_writer, test_case2));
}

TEST_SUITE_END();
