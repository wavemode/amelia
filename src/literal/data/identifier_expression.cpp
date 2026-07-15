#include "identifier_expression.hpp"

#include "binding/data/binding.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize IdentifierExpression::serialize() const {
  Serialize result;
  result.set_object_name("IdentifierExpression");
  result.add_object_field("name", Serialize::quoted(binding->name));
  return result;
}

} // namespace amelia
