#include "binding.hpp"

namespace amelia {

Serialize Binding::serialize() const {
  auto result = Serialize();
  result.set_object_name("Binding");
  result.add_object_field("name", Serialize::quoted(name));
  result.add_object_field("kind", serialize_binding_kind(kind));
  result.add_object_field("visibility", serialize_declaration_visibility(visibility));
  switch (kind) {
  case BindingKind::Variable:
  case BindingKind::Constant:
  case BindingKind::Function: {
    const ValueBinding &value_binding = static_cast<const ValueBinding &>(*this);
    if (value_binding.type.has_value()) {
      result.add_object_field("type", value_binding.type.value()->serialize());
    }
    if (value_binding.value.has_value()) {
      result.add_object_field("value", value_binding.value.value()->serialize());
    }
    break;
  }
  default:
    throw RuntimeError("not implemented");
  }
  return result;
}

Serialize serialize_binding_kind(BindingKind kind) {
  String result;
  switch (kind) {
  case BindingKind::Variable:
    result.append("Variable");
    break;
  case BindingKind::Constant:
    result.append("Constant");
    break;
  case BindingKind::Function:
    result.append("Function");
    break;
  case BindingKind::Type:
    result.append("Type");
    break;
  case BindingKind::Class:
    result.append("Class");
    break;
  case BindingKind::Concept:
    result.append("Concept");
    break;
  case BindingKind::Module:
    result.append("Module");
    break;
  }
  return Serialize::literal(move(result));
}

} // namespace amelia
