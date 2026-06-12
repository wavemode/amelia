#include "expression.hpp"

namespace amelia {

PrettyPrint NumberLiteralExpression::pretty_print() const {
  String result;
  result.append("NumberLiteralExpression(");
  pretty_print_number_literal(value).to_string(result);
  result.append(")");
  return PrettyPrintString(move(result));
}

} // namespace amelia
