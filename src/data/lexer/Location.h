#pragma once

#include <cstddef>

#include "data/lexer/TokenType.h"
#include "util/slice/Slice.h"

namespace amelia {

struct Location {
  size_t file_id;
  size_t line;
  size_t column;
};

} // namespace amelia
