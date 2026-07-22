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
      parameters_list.add_list_item(serialize_function_parameter(parameter));
    }
    result.add_object_field("parameters", move(parameters_list));
  }
  if (signature.implicit_parameters.has_value() &&
      signature.implicit_parameters.value().size() > 0) {
    Serialize implicit_parameters_list;
    for (const auto &parameter : signature.implicit_parameters.value()) {
      implicit_parameters_list.add_list_item(serialize_function_parameter(parameter));
    }
    result.add_object_field("implicit_parameters", move(implicit_parameters_list));
  }
  result.add_object_field("return_type", signature.return_type->serialize());
  return result;
}

} // namespace amelia
