#pragma once

#include <cstdint>

#include "util/slice/Slice.h"

namespace amelia {

enum class TokenType {
  IDENTIFIER,
  INTEGER,
  FLOAT,
  ASSIGN,
};

} // namespace amelia
