#pragma once

#include <vector>

#include "interface/core/IPrinter.h"
#include "util/text/String.h"

namespace amelia {

class FilesystemWalker {
public:
  FilesystemWalker();

  void walk(const String &root, std::vector<String> &output, bool regular_files_only = true,
            bool ignore_errors = true);
};

} // namespace amelia
