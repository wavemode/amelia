#pragma once

#include "interface/fs/filesystem_walker.h"

namespace amelia {

class FilesystemWalker : public IFilesystemWalker {
public:
  void walk(
      AbstractList<String> &output,
      Text root,
      bool regular_files_only = true,
      bool ignore_errors = true
  ) override;
};

} // namespace amelia
