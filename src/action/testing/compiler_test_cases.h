#pragma once

#include "prelude.h"

namespace amelia {

class IFilesystemWalker;
class IFileLoader;
class IFileWriter;
class IPrinter;
class IEnvironmentReader;
class ITestCaseRunner;
class CompilerTestCase;
class CompilerTestCaseCollection;
class CompilerTestExecutionOutcome;

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

void run_lexer_test_case(IString &output, CompilerTestCase test_case);

} // namespace amelia