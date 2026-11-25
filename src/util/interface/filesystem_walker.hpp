#pragma once

namespace amelia {

class String;
class Text;
template <typename T> class List;

struct IFilesystemWalker {
  virtual void walk(
      List<String> &output, Text root, bool regular_files_only = true,
      bool ignore_errors = true
  ) = 0;
};

} // namespace amelia
