#include <doctest.h>

#include "prelude.hpp"

#include "testing/action/compiler_test_cases.hpp"
#include "testing/data/compiler_test_case_collection.hpp"
#include "testing/data/compiler_test_case_outcome.hpp"
#include "util/data/text_utils.hpp"
#include "fs/effect/file_loader.hpp"
#include "fs/effect/file_writer.hpp"
#include "fs/effect/filesystem_walker.hpp"
#include "sys/effect/console_printer.hpp"
#include "sys/effect/environment_reader.hpp"
#include "testing/system/sema_test_case_runner.hpp"

TEST_SUITE_BEGIN("Analyzer");

using namespace amelia;

TEST_CASE("test suite") {
  FileLoader file_loader;
  FileWriter file_writer;
  ConsolePrinter console_printer;
  FilesystemWalker filesystem_walker;
  SemaTestCaseRunner sema_test_case_runner(file_loader);
  EnvironmentReader env_reader;

  CompilerTestCaseCollection collection;
  collect_test_cases(filesystem_walker, file_loader, collection, "test_cases/sema");
  auto outcome = execute_collection(
      sema_test_case_runner, file_writer, console_printer, env_reader, collection
  );
  console_printer.print("Executed ");
  String s;
  TextUtils::to_string(s, outcome.count_executed);
  console_printer.print(s);
  console_printer.print(" sema test cases with ");
  s.clear();
  TextUtils::to_string(s, outcome.count_failed);
  console_printer.print(s);
  console_printer.print(" failures and ");
  s.clear();
  TextUtils::to_string(s, outcome.count_updated);
  console_printer.print(s);
  console_printer.println(" updates.");

  REQUIRE(outcome.count_failed == 0);
}

TEST_SUITE_END();
