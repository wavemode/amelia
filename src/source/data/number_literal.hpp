#pragma once

#include "util/data/text.hpp"

namespace amelia {

class Serialize;

struct NumberLiteral {
  bool has_decimal_point;
  Text base_prefix;
  Text integer_digits;
  Text fractional_digits;
  Text exponent_prefix;
  Text exponent_sign;
  Text exponent_digits;
};

Serialize serialize_number_literal(const NumberLiteral &number_literal);

} // namespace amelia
