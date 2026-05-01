#pragma once

#include "data/core/string.h"

namespace amelia {

struct StringLiteral {
  static void read(AbstractString &output, CharIterator &iter, bool is_raw);
};

} // namespace amelia
