#include "type_binding_statement.hpp"

#include "sema/data/binding.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize TypeBindingExpression::serialize() const {
  Serialize result;
  result.set_object_name("TypeBindingExpression");
  result.add_object_field("name", Serialize::quoted(name));
  result.add_object_field("binding", binding->serialize());
  result.add_object_field("body", body->serialize());
  return result;
}

} // namespace amelia
