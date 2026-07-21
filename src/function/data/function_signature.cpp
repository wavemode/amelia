#include "function_signature.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize serialize_function_signature(const FunctionSignature &signature) {
  Serialize result;
  result.set_object_name("FunctionSignature");
  if (signature.parameters.size() > 0) {
    Serialize parameters_list;
    for (const auto &parameter : signature.parameters) {
      Serialize parameter_ser;
      parameter_ser.set_object_name("FunctionParameter");
      parameter_ser.add_object_field("name", Serialize::quoted(parameter.name));
      parameter_ser.add_object_field("type", parameter.type->serialize());
      if (parameter.default_value.has_value()) {
        parameter_ser.add_object_field(
            "default_value", parameter.default_value.value()->serialize()
        );
      }
      parameters_list.add_list_item(move(parameter_ser));
    }
    result.add_object_field("parameters", move(parameters_list));
  }
  result.add_object_field("return_type", signature.return_type->serialize());
  return result;
}

} // namespace amelia
