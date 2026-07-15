#include "type_cast_expression.hpp"

#include "type/data/type.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize TypeCastExpression::serialize() const {
  Serialize result;
  result.set_object_name("TypeCastOperationExpression");
  result.add_object_field("expr", expr->serialize());
  result.add_object_field("type", type->serialize());
  return result;
}

} // namespace amelia
