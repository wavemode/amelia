#include <cstdint>
#include <string>

#include "number_literal.h"
#include "prelude.h"

#include "data/util/text_utils.h"

namespace amelia {
bool NumberLiteral::operator==(const NumberLiteral &other) const noexcept {
  return has_decimal_point == other.has_decimal_point && base_prefix == other.base_prefix &&
         integer_digits == other.integer_digits && fractional_digits == other.fractional_digits &&
         exponent_prefix == other.exponent_prefix && exponent_sign == other.exponent_sign &&
         exponent_digits == other.exponent_digits;
}

bool NumberLiteral::operator!=(const NumberLiteral &other) const noexcept {
  return !(*this == other);
}

} // namespace amelia
