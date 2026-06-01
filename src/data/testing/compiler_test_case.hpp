#pragma once

#include <cstddef>

#include "data/util/text.hpp"

namespace amelia {

const Text COMPILER_TEST_CASE_EXPECTED_OUTPUT_HEADER = "/* EXPECTED_OUTPUT:\n";
const Text COMPILER_TEST_CASE_EXPECTED_OUTPUT_FOOTER = "*/";

struct CompilerTestCase {
  Text filename;
  Text input;
  Text expected_output;
};

} // namespace amelia
