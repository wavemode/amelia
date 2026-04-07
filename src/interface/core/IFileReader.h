#pragma once

namespace amelia {

class String;

class IFileReader {
public:
  virtual String read_file(const String &filePath) = 0;
};

} // namespace amelia
