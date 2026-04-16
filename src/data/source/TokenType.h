#pragma once

#include "util/slice/Slice.h"

namespace amelia {

enum class TokenType {
  IDENTIFIER,
  INTEGER,
  FLOAT,

  ASSIGN,
  EQUAL,

  END_OF_FILE,
};

} // namespace amelia
