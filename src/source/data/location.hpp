#pragma once

#include <cstddef>

#include "util/data/char_iterator.hpp"
#include "util/data/text.hpp"

namespace amelia {

struct Location {
  Text filename;
  CharIterator position;
  uint32_t line;
  uint32_t column;

  bool operator==(const Location &other) const {
    return filename == other.filename && line == other.line && column == other.column;
  }

  bool operator!=(const Location &other) const {
    return !(*this == other);
  }
};

} // namespace amelia
