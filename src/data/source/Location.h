#pragma once

#include <cstddef>

#include "data/source/TokenType.h"
#include "util/slice/Slice.h"

namespace amelia {

struct Location {
  size_t file_id;
  size_t line;
  size_t column;

  bool operator==(const Location &other) const {
    return file_id == other.file_id && line == other.line && column == other.column;
  }

  bool operator!=(const Location &other) const { return !(*this == other); }
};

} // namespace amelia
