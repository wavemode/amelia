#include "native_type_cast_expression.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize NativeTypeCastExpression::serialize() const {
  Serialize result;
  result.set_object_name("NativeTypeCastExpression");
  result.add_object_field("type", type->serialize());
  result.add_object_field("expr", expr->serialize());
  return result;
}

} // namespace amelia
