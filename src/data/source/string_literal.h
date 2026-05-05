#pragma once

#include <cstddef>

namespace amelia {

struct StringLiteral {
  size_t buffer_offset;
  size_t length;
};

} // namespace amelia
