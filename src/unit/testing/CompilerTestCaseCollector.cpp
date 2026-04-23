#include <cstddef>

#include "CompilerTestCaseCollector.h"

#include "interface/core/IFileLoader.h"
#include "interface/core/IFilesystemWalker.h"
#include "interface/core/IString.h"

#include "data/core/CharIterator.h"
#include "data/core/String.h"
#include "data/testing/CompilerTestCase.h"
#include "data/testing/CompilerTestCaseCollection.h"
#include "data/testing/CompilerTestCaseError.h"

namespace amelia {

namespace {

const Text EXPECTED_OUTPUT_HEADER = "/* EXPECTED_OUTPUT:\n";

Text find_test_case_expected_output(const String &file_contents) {
  auto iter = CharIterator(file_contents);
  auto start = iter.find("/* EXPECTED OUTPUT:");
  if (start.at_end()) {
    return Text();
  }
  auto text_start = start.plus(EXPECTED_OUTPUT_HEADER.size());
  auto text_end = text_start.find("*/");
  if (text_end.at_end()) {
    return Text();
  }
  return text_start.head(text_end);
}

} // namespace

CompilerTestCaseCollector::CompilerTestCaseCollector(
    IFilesystemWalker *filesystem_walker, IFileLoader *file_loader
)
    : filesystem_walker(filesystem_walker), file_loader(file_loader) {}

void CompilerTestCaseCollector::collect_test_cases(
    const String &root_directory, CompilerTestCaseCollection &output
) {
  size_t num_files_before = output.paths.size();
  filesystem_walker->walk(root_directory, output.paths);
  for (size_t i = num_files_before; i < output.paths.size(); ++i) {
    const String &path = output.paths[i];

    String &file_content = output.file_contents.emplace_back();
    file_loader->load_file(path, file_content);

    Text expected_output = find_test_case_expected_output(file_content);
    if (expected_output.size() == 0) {
      throw CompilerTestCaseError("Malformed test case: missing EXPECTED_OUTPUT comment");
    }

    output.test_cases.push_back(CompilerTestCase{Text(path), Text(file_content), expected_output});
  }
}

} // namespace amelia
