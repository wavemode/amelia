#pragma once

#include <cstddef>

namespace amelia {

class String;
using tokenized_file_id = size_t;

class ISourceFileManager {
public:
  virtual tokenized_file_id store_source_file(String) = 0;
  virtual const String &get_source_file(tokenized_file_id) = 0;
};

} // namespace amelia
