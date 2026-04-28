#pragma once

#include <cstddef>

#include "data/text/CharIterator.h"
#include "data/text/Text.h"

namespace amelia {

struct Location {
  Text filename;
  CharIterator position;
  size_t line;
  size_t column;

  bool operator==(const Location &other) const {
    return filename == other.filename && line == other.line && column == other.column;
  }

  bool operator!=(const Location &other) const { return !(*this == other); }
};

} // namespace amelia
