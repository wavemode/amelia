#include <cstddef>

#include "CompilerTestCaseCollector.h"
#include "Prelude.h"

#include "interface/fs/IFileLoader.h"
#include "interface/fs/IFilesystemWalker.h"

#include "data/core/ListUtils.h"
#include "data/testing/CompilerTestCase.h"
#include "data/testing/CompilerTestCaseCollection.h"
#include "data/testing/CompilerTestCaseError.h"
#include "data/text/TextUtils.h"

namespace amelia {

namespace {
Text find_test_case_expected_output(const String &file_contents) {
  auto expected_output_section_start =
      TextUtils::find(file_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER);
  if (expected_output_section_start.at_end()) {
    return Text();
  }
  auto text_start =
      expected_output_section_start.plus_bytes(COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER.size());
  auto text_end =
      TextUtils::find_after(file_contents, COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER, text_start);
  if (text_end.at_end()) {
    return Text();
  }
  return TextUtils::substr(file_contents, text_start, text_end);
}

} // namespace

CompilerTestCaseCollector::CompilerTestCaseCollector(
    IFilesystemWalker *filesystem_walker, IFileLoader *file_loader
)
    : filesystem_walker(filesystem_walker), file_loader(file_loader) {}

void CompilerTestCaseCollector::collect_test_cases(
    CompilerTestCaseCollection &output, const String &root_directory
) {
  size_t num_files_before = output.paths.size();
  filesystem_walker->walk(output.paths, root_directory);
  for (size_t i = num_files_before; i < output.paths.size(); ++i) {
    const String &path = output.paths[i];
    if (!TextUtils::ends_with(path, ".am")) {
      continue;
    }

    String &file_content = output.file_contents.emplace_back();
    file_loader->load_file(file_content, path);

    Text expected_output = find_test_case_expected_output(file_content);
    output.test_cases.push_back(CompilerTestCase{path, file_content, expected_output});
  }
  ListUtils::sort(output.test_cases, [](const CompilerTestCase &a, const CompilerTestCase &b) {
    return a.filename < b.filename;
  });
}

} // namespace amelia
