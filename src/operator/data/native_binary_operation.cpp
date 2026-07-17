#include "native_binary_operation.hpp"

#include "util/data/string.hpp"

namespace amelia {

Serialize NativeBinaryOperationExpression::serialize() const {
  Serialize result;
  result.set_object_name("NativeBinaryOperationExpression");
  result.add_object_field("left", left->serialize());
  result.add_object_field("op_kind", serialize_binary_operator_kind(op_kind).quoted());
  result.add_object_field("right", right->serialize());
  return result;
}

} // namespace amelia
