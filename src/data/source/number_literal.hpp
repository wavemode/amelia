#pragma once

#include "data/testing/pretty_print.hpp"

namespace amelia {

struct NumberLiteral {
  bool has_decimal_point;
  Text base_prefix;
  Text integer_digits;
  Text fractional_digits;
  Text exponent_prefix;
  Text exponent_sign;
  Text exponent_digits;
};

PrettyPrint pretty_print_number_literal(const NumberLiteral &number_literal);

} // namespace amelia
