#pragma once

#include <cstddef>

#include "util/slice/Slice.h"
#include "util/text/Text.h"

namespace amelia {

struct CompilerTestCase {
  Text filename;
  Text input;
  Text expected_output;
};

} // namespace amelia
