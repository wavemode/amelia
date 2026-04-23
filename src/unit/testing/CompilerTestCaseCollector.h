#pragma once

#include "data/core/List.h"
#include "data/testing/CompilerTestCaseCollection.h"

namespace amelia {

class IFilesystemWalker;
class IFileLoader;
class CompilerTestCase;
class String;

class CompilerTestCaseCollector {
public:
  CompilerTestCaseCollector(IFilesystemWalker *filesystem_walker, IFileLoader *file_loader);

  void collect_test_cases(CompilerTestCaseCollection &output, const String &root_directory);

private:
  IFilesystemWalker *filesystem_walker;
  IFileLoader *file_loader;
};

} // namespace amelia
