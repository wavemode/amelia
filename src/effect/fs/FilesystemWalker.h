#pragma once

#include "interface/fs/IFilesystemWalker.h"

namespace amelia {

class FilesystemWalker : public IFilesystemWalker {
public:
  void walk(
      IList<String> &output, Text root, bool regular_files_only = true, bool ignore_errors = true
  ) override;
};

} // namespace amelia
