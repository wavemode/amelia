#include <cstddef>

#include "compiler_test_cases.h"

#include "interface/fs/file_loader.h"
#include "interface/fs/file_writer.h"
#include "interface/fs/filesystem_walker.h"
#include "interface/sys/environment_reader.h"
#include "interface/sys/printer.h"
#include "interface/testing/test_case_runner.h"

#include "data/lexer/lexer.h"
#include "data/lexer/lexer_context.h"
#include "data/lexer/lexer_result.h"
#include "data/lexer/token.h"
#include "data/parser/parser.h"
#include "data/parser/parser_result.h"
#include "data/source/source_location_error.h"
#include "data/testing/compiler_test_case.h"
#include "data/testing/compiler_test_case_collection.h"
#include "data/testing/compiler_test_case_outcome.h"

#include "data/util/text_utils.h"

namespace amelia {

namespace {
Text find_test_case_expected_output(const String &file_contents) {
  auto expected_output_section_start = TextUtils::find(
      file_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER
  );
  if (expected_output_section_start.at_end()) {
    return Text();
  }
  auto text_start = expected_output_section_start.plus_bytes(
      COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER.size()
  );
  auto text_end = TextUtils::find_after(
      file_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER, text_start
  );
  if (text_end.at_end()) {
    return Text();
  }
  return TextUtils::substr(file_contents, text_start, text_end);
}

void update_file_expected_output(
    IFileWriter &file_writer, Text path, Text existing_contents, Text expected_output
) {
  String new_file_contents;
  auto header = TextUtils::find(existing_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER);

  new_file_contents.append(TextUtils::head(existing_contents, header));
  new_file_contents.append(COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER);
  new_file_contents.append(expected_output);
  new_file_contents.append(COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER);

  auto footer = TextUtils::find_after(
      existing_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER, header
  );
  auto tail_section = footer.at_end()
                          ? footer
                          : footer.plus_bytes(COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER.size());

  auto tail_text = TextUtils::tail(existing_contents, tail_section);
  if (tail_text == "") {
    tail_text = "\n";
  }
  new_file_contents.append(tail_text);

  file_writer.write_file(String(path), new_file_contents);
}

} // namespace

void collect_test_cases(
    IFilesystemWalker &filesystem_walker,
    IFileLoader &file_loader,
    CompilerTestCaseCollection &output,
    Text root_directory
) {
  size_t num_files_before = output.paths.size();
  filesystem_walker.walk(output.paths, root_directory);
  for (size_t i = num_files_before; i < output.paths.size(); ++i) {
    const String &path = output.paths[i];
    if (!TextUtils::ends_with(path, ".am")) {
      continue;
    }

    String &file_content = output.file_contents.emplace_back();
    file_loader.load_file(file_content, path);

    Text expected_output = find_test_case_expected_output(file_content);
    output.test_cases.push_back(CompilerTestCase{path, file_content, expected_output});
  }
  output.test_cases.sort([](const CompilerTestCase &a, const CompilerTestCase &b) {
    return a.filename < b.filename;
  });
}

CompilerTestExecutionOutcome execute_collection(
    ITestCaseRunner &test_case_runner,
    IFileWriter &file_writer,
    IPrinter &printer,
    IEnvironmentReader &env_reader,
    const CompilerTestCaseCollection &collection
) {
  bool update_test_cases = true;
  String env_value;
  env_reader.get_env(env_value, String("AMELIA_UPDATE_TEST_CASES"));
  if (env_value == "" || env_value == "0") {
    update_test_cases = false;
  }

  String test_case_filter;
  env_reader.get_env(test_case_filter, String("AMELIA_TEST_CASE_FILTER"));
  bool has_filter = test_case_filter != "";

  size_t num_executed = 0;
  size_t num_failed = 0;
  for (const CompilerTestCase &test_case : collection.test_cases) {
    if (has_filter && !TextUtils::contains(test_case.filename, test_case_filter)) {
      continue;
    }
    ++num_executed;
    bool should_error = TextUtils::contains(test_case.filename, "error_");
    String actual_output;
    try {
      test_case_runner.run_test_case(actual_output, test_case);
      if (should_error) {
        printer.print("Expected test case to raise an exception but it raised none: ");
        printer.println(test_case.filename);
        ++num_failed;
      }
    } catch (const SourceLocationError &e) {
      if (!should_error) {
        printer.print("Test case threw an unexpected exception: ");
        printer.print(" (error message: \"");
        printer.print(Text::from(e.what()));
        printer.println("\")");
        ++num_failed;
      }
      actual_output.append("ERROR(\"");
      actual_output.append(Text::from(e.what()));
      actual_output.append("\")\n");
    }

    if (actual_output.text() != test_case.expected_output) {
      if (update_test_cases) {
        update_file_expected_output(
            file_writer, test_case.filename, test_case.input, actual_output
        );
        printer.print("Updated expected output for: ");
        printer.println(test_case.filename);
      } else {
        printer.print("Test case failed: ");
        printer.println(test_case.filename);
        ++num_failed;
      }
    }
  }
  return {
      .count_executed = num_executed,
      .count_failed = num_failed,
  };
}

bool execute_test_case(ITestCaseRunner &test_case_runner, CompilerTestCase test_case) {
  String actual_output;
  test_case_runner.run_test_case(actual_output, test_case);
  return actual_output.text() != test_case.expected_output;
}

bool update_expected_output(
    ITestCaseRunner &test_case_runner, IFileWriter &file_writer, CompilerTestCase test_case
) {
  String actual_output;
  test_case_runner.run_test_case(actual_output, test_case);
  if (actual_output.text() == test_case.expected_output) {
    return false;
  }

  update_file_expected_output(file_writer, test_case.filename, test_case.input, actual_output);

  return true;
}

void run_lexer_test_case(AbstractString &output, CompilerTestCase test_case) {
  LexerResult result;
  Lexer::tokenize(result, LexerContext{test_case.filename}, test_case.input);
  auto tokens = result.tokens();
  for (size_t token_id = 0; token_id < tokens.size(); ++token_id) {
    result.format_token(output, token_id);
    output.append("\n");
  }
}

void run_parser_test_case(AbstractString &output, CompilerTestCase test_case) {
  LexerResult lexer_result;
  Lexer::tokenize(lexer_result, LexerContext{test_case.filename}, test_case.input);
  ParserResult parser_result;
  NodeId root_node_id = Parser::parse_module(parser_result, lexer_result);
  parser_result.format_node(output, lexer_result, root_node_id);
  output.append("\n");
}

} // namespace amelia
