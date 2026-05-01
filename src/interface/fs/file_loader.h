#pragma once

namespace amelia {

class IString;

struct IFileLoader {
  virtual void load_file(IString &output, const IString &file_path) = 0;
};

} // namespace amelia
