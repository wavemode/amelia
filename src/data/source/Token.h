#pragma once

#include <cstdint>

#include "data/source/Location.h"
#include "data/source/TokenType.h"
#include "util/slice/Slice.h"
#include "util/text/Text.h"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;

  bool operator==(const Token &other) const {
    return type == other.type && location == other.location && contents == other.contents;
  }

  bool operator!=(const Token &other) const { return !(*this == other); }
};

} // namespace amelia
