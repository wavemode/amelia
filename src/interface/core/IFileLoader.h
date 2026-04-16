#pragma once

namespace amelia {

class String;
class Text;

class IFileLoader {
public:
  virtual String load_file(const Text &file_path) = 0;
};

} // namespace amelia
