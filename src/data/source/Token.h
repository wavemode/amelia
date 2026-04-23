#pragma once

#include "data/core/TextUtils.h"
#include "data/source/Location.h"
#include "data/source/TokenType.h"

namespace amelia {

struct Token {
  TokenType type;
  Location location;
  Text contents;

  bool operator==(const Token &other) const {
    return type == other.type && location == other.location && contents == other.contents;
  }

  bool operator!=(const Token &other) const { return !(*this == other); }

  void to_string(IString &out) const {
    token_type_to_string(type, out);
    out.append("(\"");
    out.append(contents);
    out.append("\"");
    out.append(")");
  }
};

} // namespace amelia
