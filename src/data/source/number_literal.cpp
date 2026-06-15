#include "number_literal.hpp"

namespace amelia {
PrettyPrint pretty_print_number_literal(const NumberLiteral &number_literal) {
  String result;
  result.append(number_literal.base_prefix);
  result.append(number_literal.integer_digits);
  if (number_literal.has_decimal_point) {
    result.append(".");
    result.append(number_literal.fractional_digits);
  }
  result.append(number_literal.exponent_prefix);
  result.append(number_literal.exponent_sign);
  result.append(number_literal.exponent_digits);
  return PrettyPrint::literal(move(result));
}
} // namespace amelia