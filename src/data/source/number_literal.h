#pragma once

#include "data/util/text.h"

namespace amelia {

struct NumberLiteral {
  bool has_decimal_point;
  Text base_prefix;
  Text integer_digits;
  Text fractional_digits;
  Text exponent_prefix;
  Text exponent_sign;
  Text exponent_digits;

  bool operator==(const NumberLiteral &other) const noexcept;
  bool operator!=(const NumberLiteral &other) const noexcept;

  static NumberLiteral read(CharIterator &iter);
};

} // namespace amelia
