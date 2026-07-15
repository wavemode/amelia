#include "tuple_expression.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize TupleExpression::serialize() const {
  Serialize result;
  result.set_object_name("TupleExpression");
  Serialize elements_ser;
  for (const auto &element : elements) {
    elements_ser.add_list_item(element->serialize());
  }
  result.add_object_field("elements", move(elements_ser));
  return result;
}

} // namespace amelia
