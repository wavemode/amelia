#include <string>

#include <doctest.h>

#include "testing/data/compiler_test_case_collection.hpp"
#include "testing/data/compiler_test_case_outcome.hpp"
#include "testing/data/compiler_test_execution_context.hpp"
#include "testing/logic/compiler_test_cases.hpp"
#include "testing/system/parser_test_case_runner.hpp"
#include "util/data/text_utils.hpp"
#include "util/effect/console_printer.hpp"
#include "util/effect/environment_reader.hpp"
#include "util/effect/file_loader.hpp"
#include "util/effect/file_writer.hpp"
#include "util/effect/filesystem_walker.hpp"

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
  collect_test_cases(
      filesystem_walker, file_loader, console_printer, env_reader, collection, "test_cases/parser"
  );

  CompilerTestExecutionContext execution_context = get_test_execution_context(env_reader);
  CompilerTestExecutionOutcome outcome;
  for (const auto &test_case : collection.test_cases) {
    INFO(std::string(test_case.filename.data().ptr(), test_case.filename.data().size()));
    CHECK(execute_test_case(
        parser_test_case_runner, file_writer, console_printer, outcome, test_case, execution_context
    ));
  }

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
  console_printer.println(" updates.");

  CHECK(outcome.count_failed == 0);
}

TEST_SUITE_END();

#include <string>
#include <type_traits>

// Non-standard-layout type (std::string contains non-standard layout internals/pointers/access
// controls)
struct NonStandard {
  std::string text;
};

// Contains a field that is NOT standard-layout
struct Container {
  int x;
  NonStandard field; // Breaks standard-layout rule
};

static_assert(std::is_standard_layout_v<NonStandard>);
static_assert(std::is_standard_layout_v<Container>); // Container is NOT standard-layout
