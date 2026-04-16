#pragma once

#include <cstddef>

namespace amelia {

class String;
using FileId = size_t;

class ISourceFileManager {
public:
  virtual FileId store_source_file(String) = 0;
  virtual const String &get_source_file(FileId) = 0;
};

} // namespace amelia
