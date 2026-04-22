#pragma once

namespace amelia {

class IString;

class IFileLoader {
public:
  virtual void load_file(const IString &file_path, IString &output) = 0;
};

} // namespace amelia
