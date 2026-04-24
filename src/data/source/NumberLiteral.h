#pragma once

#include "data/text/Text.h"

namespace amelia {

struct NumberLiteral {
  bool has_decimal_point;
  Text base_prefix;
  Text integer_digits;
  Text fractional_digits;
  Text exponent_prefix;
  Text exponent_sign;
  Text exponent_digits;

  static NumberLiteral read(Text text);
};

} // namespace amelia
