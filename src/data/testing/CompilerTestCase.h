#pragma once

#include <cstddef>

#include "data/core/Slice.h"
#include "data/core/Text.h"

namespace amelia {

struct CompilerTestCase {
  Text filename;
  Text input;
  Text expected_output;
};

} // namespace amelia
