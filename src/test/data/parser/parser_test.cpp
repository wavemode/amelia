#include <doctest.h>

#include "prelude.h"

#include "action/testing/compiler_test_cases.h"
#include "data/testing/compiler_test_case_collection.h"
#include "data/testing/compiler_test_case_outcome.h"
#include "data/util/text_utils.h"
#include "effect/fs/file_loader.h"
#include "effect/fs/file_writer.h"
#include "effect/fs/filesystem_walker.h"
#include "effect/sys/console_printer.h"
#include "effect/sys/environment_reader.h"
#include "system/testing/parser_test_case_runner.h"

TEST_SUITE_BEGIN("Parser");

using namespace amelia;

TEST_CASE("test suite") {
  FileLoader file_loader;
  FileWriter file_writer;
  ConsolePrinter console_printer;
  FilesystemWalker filesystem_walker;
  ParserTestCaseRunner parser_test_case_runner;
  EnvironmentReader env_reader;

  CompilerTestCaseCollection collection;
  collect_test_cases(filesystem_walker, file_loader, collection, "test_cases/parser");
  auto outcome = execute_collection(
      parser_test_case_runner, file_writer, console_printer, env_reader, collection
  );
  console_printer.print("Executed ");
  String s;
  TextUtils::to_string(s, outcome.count_executed);
  console_printer.print(s);
  console_printer.print(" parser test cases with ");
  s.clear();
  TextUtils::to_string(s, outcome.count_failed);
  console_printer.print(s);
  console_printer.print(" failures and ");
  s.clear();
  TextUtils::to_string(s, outcome.count_updated);
  console_printer.print(s);
  console_printer.println(" updated.");

  CHECK(outcome.count_failed == 0);
}

TEST_SUITE_END();
