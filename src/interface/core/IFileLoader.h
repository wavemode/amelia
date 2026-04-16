#pragma once

namespace amelia {

class String;

class IFileLoader {
public:
  virtual String load_file(const String &file_path) = 0;
};

} // namespace amelia
