#pragma once

#include <cstddef>

#include "data/core/text.h"

namespace amelia {

const Text COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER = "/* EXPECTED_OUTPUT:\n";
const Text COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER = "*/";

struct CompilerTestCase {
  Text filename;
  Text input;
  Text expected_output;
};

} // namespace amelia
