#include "native_unary_operation.hpp"

#include "native_unary_operation.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize NativeUnaryOperationExpression::serialize() const {
  Serialize result;
  result.set_object_name("NativeUnaryOperationExpression");
  result.add_object_field("operator", serialize_unary_operator_kind(op_kind).quoted());
  result.add_object_field("operand", operand->serialize());
  return result;
}

} // namespace amelia
