#pragma once

namespace amelia {

class IString;

struct IFileLoader {
  virtual void load_file(const IString &file_path, IString &output) = 0;
};

} // namespace amelia
