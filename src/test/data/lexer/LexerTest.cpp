#include "Prelude.h"
#include <doctest.h>

#include "action/testing/compiler_test_cases.h"
#include "data/testing/CompilerTestExecutionOutcome.h"
#include "data/text/TextUtils.h"
#include "effect/fs/FileLoader.h"
#include "effect/fs/FileWriter.h"
#include "effect/fs/FilesystemWalker.h"
#include "effect/sys/ConsolePrinter.h"
#include "effect/sys/EnvironmentReader.h"
#include "unit/testing/LexerTestCaseRunner.h"

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
  collect_test_cases(filesystem_walker, file_loader, collection, "src/test/data/lexer");
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
  console_printer.println(" failures.");

  CHECK(outcome.count_failed == 0);
}

TEST_SUITE_END();
