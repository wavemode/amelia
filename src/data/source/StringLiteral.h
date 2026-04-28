#pragma once

#include "data/text/String.h"

namespace amelia {

struct StringLiteral {
  static void read(IString &output, CharIterator &iter, bool is_raw);
};

} // namespace amelia
