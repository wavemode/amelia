#include <cstddef>

#include "compiler_test_cases.hpp"

#include "lexer/data/lexer.hpp"
#include "lexer/data/lexer_context.hpp"
#include "lexer/data/lexer_result.hpp"
#include "lexer/data/token.hpp"
#include "lexer/data/token_formatter.hpp"
#include "parser/data/node_formatter.hpp"
#include "parser/data/parser.hpp"
#include "parser/data/parser_result.hpp"
#include "sema/data/analyzer.hpp"
#include "sema/data/module_loader_context.hpp"
#include "sema/data/sema_result.hpp"
#include "sema/logic/load_module.hpp"
#include "source/data/source_location_error.hpp"
#include "testing/data/compiler_test_case.hpp"
#include "testing/data/compiler_test_case_collection.hpp"
#include "testing/data/compiler_test_case_outcome.hpp"
#include "testing/interface/test_case_runner.hpp"
#include "util/data/serialize.hpp"
#include "util/data/slice_utils.hpp"
#include "util/data/text_utils.hpp"
#include "util/interface/environment_reader.hpp"
#include "util/interface/file_loader.hpp"
#include "util/interface/file_writer.hpp"
#include "util/interface/filesystem_walker.hpp"
#include "util/interface/printer.hpp"

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
    IPrinter &printer,
    IEnvironmentReader &env_reader,
    CompilerTestCaseCollection &output,
    Text root_directory
) {
  String test_case_filter;
  env_reader.get_env(test_case_filter, String("AMELIA_TEST_CASE_FILTER"));
  bool has_filter = test_case_filter != "";

  printer.print("Collecting test cases from directory: ");
  printer.println(root_directory);

  size_t num_files_before = output.paths.size();
  filesystem_walker.walk(output.paths, root_directory);
  for (size_t i = num_files_before; i < output.paths.size(); ++i) {
    const String &path = output.paths[i];
    if (!TextUtils::ends_with(path, ".am")) {
      continue;
    }

    if (has_filter && !TextUtils::contains(path, test_case_filter)) {
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

  bool verbose = true;
  env_value.clear();
  env_reader.get_env(env_value, String("AMELIA_TEST_VERBOSE"));
  if (env_value == "" || env_value == "0") {
    verbose = false;
  }

  size_t num_executed = 0;
  size_t num_failed = 0;
  size_t num_updated = 0;
  for (const CompilerTestCase &test_case : collection.test_cases) {
    if (verbose) {
      printer.print("Executing test case: ");
      printer.println(test_case.filename);
    }

    ++num_executed;
    // TODO: need more robust logic for deciding that something is supposed to error
    bool should_error = TextUtils::contains(test_case.filename, "error_");
    bool did_error = false;
    String actual_output;
    try {
      test_case_runner.run_test_case(actual_output, test_case);
      if (should_error) {
        printer.print("Expected test case to raise an exception but it raised none: ");
        printer.println(test_case.filename);
      }
    } catch (const SourceLocationError &e) {
      did_error = true;
      if (!should_error) {
        printer.print("Test case threw an unexpected exception: ");
        printer.print(" (error message: \"");
        printer.print(Text::from(e.what()));
        printer.println("\")");
      }
      actual_output.append("ERROR(\"");
      actual_output.append(Text::from(e.what()));
      actual_output.append("\")\n");
    }

    if (actual_output.text() != test_case.expected_output) {
      if (update_test_cases && (should_error == did_error)) {
        update_file_expected_output(
            file_writer, test_case.filename, test_case.input, actual_output
        );
        printer.print("Updated expected output for: ");
        printer.println(test_case.filename);
        ++num_updated;
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
      .count_updated = num_updated,
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
  TokenFormatter formatter(result);
  auto tokens = result.tokens();
  for (size_t token_id = 0; token_id < tokens.size(); ++token_id) {
    formatter.format_token(output, token_id);
    output.append("\n");
  }
}

void run_parser_test_case(AbstractString &output, CompilerTestCase test_case) {
  LexerResult lexer_result;
  Lexer::tokenize(lexer_result, LexerContext{test_case.filename}, test_case.input);
  ParserResult parser_result;
  NodeId root_node_id = Parser::parse_module(parser_result, lexer_result);
  NodeFormatter node_formatter(parser_result);
  node_formatter.format_node(output, root_node_id);
  output.append("\n");
}

void run_sema_test_case(
    IFileLoader &file_loader, AbstractString &output, CompilerTestCase test_case
) {
  List<Text> filename_parts;
  Text path_separator = TextUtils::determine_path_separator(test_case.filename);
  TextUtils::split(filename_parts, test_case.filename, path_separator);
  String module_path;
  TextUtils::join_into(
      module_path,
      SliceUtils::head(filename_parts.data(), filename_parts.size() - 1),
      path_separator
  );
  Text filename = filename_parts[filename_parts.size() - 1];
  filename_parts.clear();
  TextUtils::split(filename_parts, filename, ".");
  String module_name(filename_parts[0]);

  SemaResult sema_result;
  ModuleLoaderContext ctx{Set({module_path})};
  load_module(file_loader, sema_result, module_name, ctx);
  Analyzer::analyze(sema_result);

  sema_result.serialize().to_string(output);
  output.append("\n");
}

} // namespace amelia
