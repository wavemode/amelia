#pragma once

namespace amelia {

class String;
class Text;
template <typename T> class AbstractList;

struct IFilesystemWalker {
  virtual void walk(
      AbstractList<String> &output,
      Text root,
      bool regular_files_only = true,
      bool ignore_errors = true
  ) = 0;
};

} // namespace amelia
