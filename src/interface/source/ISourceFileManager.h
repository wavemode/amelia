#pragma once

#include <cstddef>

namespace amelia {

class String;
using file_id = size_t;

class ISourceFileManager {
public:
  virtual file_id store_source_file(String) = 0;
  virtual const String &get_source_file(file_id) = 0;
};

} // namespace amelia
