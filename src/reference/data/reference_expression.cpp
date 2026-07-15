#include "reference_expression.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize ReferenceExpression::serialize() const {
  Serialize result;
  result.set_object_name("ReferenceExpression");
  result.add_object_field("referent", referent->serialize());
  result.add_object_field("is_const", Serialize::of(is_const));
  result.add_object_field("is_move", Serialize::of(is_move));
  return result;
}

} // namespace amelia
