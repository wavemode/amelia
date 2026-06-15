#include "binding.hpp"

namespace amelia {

PrettyPrint Binding::pretty_print(String name) const {
  auto result = PrettyPrint();
  result.set_object_name("Binding");
  result.add_object_field("name", PrettyPrint::quoted(move(name)));
  result.add_object_field("kind", pretty_print_binding_kind(kind));
  result.add_object_field("visibility", pretty_print_declaration_visibility(visibility));
  switch (kind) {
  case BindingKind::Variable:
  case BindingKind::Constant:
  case BindingKind::Function: {
    const ValueBinding &value_binding = static_cast<const ValueBinding &>(*this);
    if (value_binding.type.has_value()) {
      result.add_object_field("type", value_binding.type.value()->pretty_print());
    }
    if (value_binding.value.has_value()) {
      result.add_object_field("value", value_binding.value.value()->pretty_print());
    }
    break;
  }
  default:
    throw RuntimeError("not implemented");
  }
  return result;
}

PrettyPrint pretty_print_binding_kind(BindingKind kind) {
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
  return PrettyPrint::literal(move(result));
}

} // namespace amelia
