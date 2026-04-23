#include "CompilerTestCaseExecutor.h"

#include "data/core/TextUtils.h"
#include "data/testing/CompilerTestCase.h"

namespace amelia {

namespace {
void update_file_expected_output(
    Text path, Text existing_contents, Text expected_output, IFileWriter *file_writer
) {
  String new_file_contents;
  auto header = TextUtils::find(existing_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER);

  new_file_contents.append(TextUtils::head(existing_contents, header));
  new_file_contents.append(COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER);
  new_file_contents.append(expected_output);
  new_file_contents.append(COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER);

  auto footer =
      TextUtils::find_after(existing_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER, header);
  auto tail_section = footer.at_end()
                          ? footer
                          : footer.plus_bytes(COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER.size());

  new_file_contents.append(TextUtils::tail(existing_contents, tail_section));

  file_writer->write_file(String(path), new_file_contents);
}
} // namespace

CompilerTestCaseExecutor::CompilerTestCaseExecutor(
    ITestCaseRunner *test_case_runner, IFileWriter *file_writer
)
    : test_case_runner(test_case_runner), file_writer(file_writer) {}

bool CompilerTestCaseExecutor::execute_test_case(const CompilerTestCase &test_case) {
  String actual_output;
  test_case_runner->run_test_case(test_case, actual_output);
  return actual_output.text() != test_case.expected_output;
}
bool CompilerTestCaseExecutor::update_expected_output(const CompilerTestCase &test_case) {
  String actual_output;
  test_case_runner->run_test_case(test_case, actual_output);
  if (actual_output.text() == test_case.expected_output) {
    return false;
  }

  update_file_expected_output(test_case.filename, test_case.input, actual_output, file_writer);

  return true;
}
} // namespace amelia
