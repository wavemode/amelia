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

TEST_SUITE_BEGIN("Lexer");

using namespace amelia;

TEST_CASE("test suite") {
  FileLoader file_loader;
  FileWriter file_writer;
  ConsolePrinter console_printer;
  FilesystemWalker filesystem_walker;
  LexerTestCaseRunner lexer_test_case_runner;
  EnvironmentReader env_reader;
  CompilerTestCaseExecutor executor(
      &lexer_test_case_runner, &file_writer, &console_printer, &env_reader
  );

  CompilerTestCaseCollection collection;
  CompilerTestCaseCollector collector(&filesystem_walker, &file_loader);
  collector.collect_test_cases(collection, "test_cases/lexer");

  CHECK(executor.execute_collection(collection) == 0);
}

TEST_SUITE_END();
