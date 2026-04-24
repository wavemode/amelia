#pragma once

namespace amelia {

class String;
template <typename T> class IList;

struct IFilesystemWalker {
  virtual void walk(
      IList<String> &output,
      const String &root,
      bool regular_files_only = true,
      bool ignore_errors = true
  ) = 0;
};

} // namespace amelia
