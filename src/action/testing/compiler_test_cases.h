#pragma once

#include "Prelude.h"

#include "interface/fs/IFileLoader.h"
#include "interface/fs/IFileWriter.h"
#include "interface/fs/IFilesystemWalker.h"
#include "interface/sys/IEnvironmentReader.h"
#include "interface/sys/IPrinter.h"
#include "interface/testing/ITestCaseRunner.h"

#include "data/testing/CompilerTestCaseCollection.h"
#include "data/testing/CompilerTestExecutionOutcome.h"

namespace amelia {

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