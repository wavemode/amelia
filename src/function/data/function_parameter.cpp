#include "function_parameter.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize serialize_function_parameter(const FunctionParameter &parameter) {
  Serialize parameter_ser;
  parameter_ser.set_object_name("FunctionParameter");
  parameter_ser.add_object_field("name", Serialize::quoted(parameter.name));
  parameter_ser.add_object_field("type", parameter.type->serialize());
  if (parameter.default_value.has_value()) {
    parameter_ser.add_object_field("default_value", parameter.default_value.value()->serialize());
  }
  return parameter_ser;
}

} // namespace amelia
