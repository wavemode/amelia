#pragma once

#include <cstddef>

#include "data/util/char_iterator.hpp"
#include "data/util/text.hpp"

namespace amelia {

struct Location {
  Text filename;
  CharIterator position;
  size_t line;
  size_t column;

  bool operator==(const Location &other) const {
    return filename == other.filename && line == other.line && column == other.column;
  }

  bool operator!=(const Location &other) const {
    return !(*this == other);
  }
};

} // namespace amelia
