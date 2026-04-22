#pragma once

#include "data/core/List.h"

namespace amelia {

class IFilesystemWalker;
class IFileLoader;
class CompilerTestCase;
class CompilerTestCaseCollection;
class String;

class CompilerTestCaseCollector {
public:
  CompilerTestCaseCollector(IFilesystemWalker *filesystem_walker, IFileLoader *file_loader);

  void collect_test_cases(const String &root_directory, CompilerTestCaseCollection &output);

private:
  IFilesystemWalker *filesystem_walker;
  IFileLoader *file_loader;
};

} // namespace amelia
