#include "expression.hpp"

namespace amelia {

PrettyPrint NumberLiteralExpression::pretty_print() const {
  PrettyPrint result;
  result.set_tuple_name("NumberLiteralExpression");
  result.add_tuple_item(pretty_print_number_literal(value));
  return result;
}

} // namespace amelia
