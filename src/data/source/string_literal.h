#pragma once

#include "data/util/string.h"

namespace amelia {

struct StringLiteral {
  static void read(AbstractString &output, CharIterator &iter, bool is_raw);
};

} // namespace amelia
