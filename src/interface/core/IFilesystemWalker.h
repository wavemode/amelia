#pragma once

#include "data/core/List.h"

namespace amelia {

class String;

struct IFilesystemWalker {
  virtual void walk(
      const String &root,
      List<String> &output,
      bool regular_files_only = true,
      bool ignore_errors = true
  ) = 0;
};

} // namespace amelia
