#pragma once

#include "prelude.hpp"

namespace amelia {

struct IFilesystemWalker;
struct IFileLoader;
struct IFileWriter;
struct IPrinter;
struct IEnvironmentReader;
struct ITestCaseRunner;
struct CompilerTestCase;
struct CompilerTestCaseCollection;
struct CompilerTestExecutionOutcome;

void collect_test_cases(
    IFilesystemWalker &, IFileLoader &, CompilerTestCaseCollection &output, Text root_directory
);

CompilerTestExecutionOutcome execute_collection(
    ITestCaseRunner &,
    IFileWriter &,
    IPrinter &,
    IEnvironmentReader &,
    const CompilerTestCaseCollection &collection
);

bool execute_test_case(ITestCaseRunner &, CompilerTestCase test_case);

bool update_expected_output(ITestCaseRunner &, IFileWriter &, CompilerTestCase test_case);

void run_lexer_test_case(AbstractString &output, CompilerTestCase test_case);

void run_parser_test_case(AbstractString &output, CompilerTestCase test_case);

void run_sema_test_case(
    IFileLoader &file_loader, AbstractString &output, CompilerTestCase test_case
);

} // namespace amelia