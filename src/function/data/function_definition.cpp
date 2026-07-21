#include "function_definition.hpp"

#include "function/data/function_signature.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize serialize_function_definition(const FunctionDefinition &definition) {
  Serialize result;
  result.set_object_name("FunctionDefinition");
  result.add_object_field("signature", serialize_function_signature(definition.signature));
  result.add_object_field("body", definition.body->serialize());
  return result;
}

} // namespace amelia
