#include <doctest.h>

#include "prelude.hpp"

#include "action/testing/compiler_test_cases.hpp"
#include "data/testing/compiler_test_case_collection.hpp"
#include "data/testing/compiler_test_case_outcome.hpp"
#include "data/util/text_utils.hpp"
#include "effect/fs/file_loader.hpp"
#include "effect/fs/file_writer.hpp"
#include "effect/fs/filesystem_walker.hpp"
#include "effect/sys/console_printer.hpp"
#include "effect/sys/environment_reader.hpp"
#include "system/testing/lexer_test_case_runner.hpp"

TEST_SUITE_BEGIN("Lexer");

using namespace amelia;

TEST_CASE("test suite") {
  FileLoader file_loader;
  FileWriter file_writer;
  ConsolePrinter console_printer;
  FilesystemWalker filesystem_walker;
  LexerTestCaseRunner lexer_test_case_runner;
  EnvironmentReader env_reader;

  CompilerTestCaseCollection collection;
  collect_test_cases(filesystem_walker, file_loader, collection, "test_cases/lexer");
  auto outcome = execute_collection(
      lexer_test_case_runner, file_writer, console_printer, env_reader, collection
  );
  console_printer.print("Executed ");
  String s;
  TextUtils::to_string(s, outcome.count_executed);
  console_printer.print(s);
  console_printer.print(" lexer test cases with ");
  s.clear();
  TextUtils::to_string(s, outcome.count_failed);
  console_printer.print(s);
  console_printer.print(" failures and ");
  s.clear();
  TextUtils::to_string(s, outcome.count_updated);
  console_printer.print(s);
  console_printer.println(" updates.");

  CHECK(outcome.count_failed == 0);
}

TEST_SUITE_END();
