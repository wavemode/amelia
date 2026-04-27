#include "CompilerTestCaseExecutor.h"
#include "Prelude.h"

#include "data/testing/CompilerTestCase.h"
#include "data/text/TextUtils.h"

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

  auto tail_text = TextUtils::tail(existing_contents, tail_section);
  if (tail_text == "") {
    tail_text = "\n";
  }
  new_file_contents.append(tail_text);

  file_writer->write_file(String(path), new_file_contents);
}
} // namespace

CompilerTestCaseExecutor::CompilerTestCaseExecutor(
    ITestCaseRunner *test_case_runner,
    IFileWriter *file_writer,
    IPrinter *printer,
    IEnvironmentReader *env_reader
)
    : test_case_runner(test_case_runner), file_writer(file_writer), printer(printer),
      env_reader(env_reader) {}

size_t CompilerTestCaseExecutor::execute_collection(const CompilerTestCaseCollection &collection) {
  bool update_test_cases = true;
  String env_value;
  env_reader->get_env(env_value, String("AMELIA_UPDATE_TEST_CASES"));
  if (env_value == "" || env_value == "0") {
    update_test_cases = false;
  }

  size_t num_failed = 0;
  for (const CompilerTestCase &test_case : collection.test_cases) {
    bool should_error = TextUtils::contains(test_case.filename, "error_");
    String actual_output;
    try {
      test_case_runner->run_test_case(actual_output, test_case);
      if (should_error) {
        printer->print("Expected test case to raise an exception but it raised none: ");
        printer->println(test_case.filename);
        ++num_failed;
      }
    } catch (const std::exception &e) {
      if (!should_error) {
        printer->print("Test case threw an unexpected exception: ");
        printer->print(test_case.filename);
        printer->print(" (error message: \"");
        printer->print(Text::from(e.what()));
        printer->println("\")");
        ++num_failed;
      }
      actual_output.append("ERROR(\"");
      actual_output.append(Text::from(e.what()));
      actual_output.append("\")\n");
    }

    if (actual_output.text() != test_case.expected_output) {
      if (update_test_cases) {
        update_file_expected_output(
            test_case.filename, test_case.input, actual_output, file_writer
        );
        printer->print("Updated expected output for: ");
        printer->println(test_case.filename);
      } else {
        printer->print("Test case failed: ");
        printer->println(test_case.filename);
        ++num_failed;
      }
    }
  }
  return num_failed;
}

bool CompilerTestCaseExecutor::execute_test_case(const CompilerTestCase &test_case) {
  String actual_output;
  test_case_runner->run_test_case(actual_output, test_case);
  return actual_output.text() != test_case.expected_output;
}
bool CompilerTestCaseExecutor::update_expected_output(const CompilerTestCase &test_case) {
  String actual_output;
  test_case_runner->run_test_case(actual_output, test_case);
  if (actual_output.text() == test_case.expected_output) {
    return false;
  }

  update_file_expected_output(test_case.filename, test_case.input, actual_output, file_writer);

  return true;
}
} // namespace amelia
