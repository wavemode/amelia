#pragma once

#include "util/interface/filesystem_walker.hpp"

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
