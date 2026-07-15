#include "binding.hpp"

#include "binding/data/module_binding.hpp"
#include "binding/data/type_binding.hpp"
#include "binding/data/value_binding.hpp"
#include "util/data/serialize.hpp"

namespace amelia {

Serialize Binding::serialize() const {
  auto result = Serialize();
  result.add_object_field("name", Serialize::quoted(name));
  switch (kind) {
  case BindingKind::Variable:
  case BindingKind::Constant:
  case BindingKind::Function: {
    const ValueBinding &value_binding = static_cast<const ValueBinding &>(*this);
    result.set_object_name("ValueBinding");
    if (value_binding.type.has_value()) {
      result.add_object_field("type", value_binding.type.value()->serialize());
    }
    if (value_binding.value.has_value()) {
      result.add_object_field("value", value_binding.value.value()->serialize());
    }
  } break;
  case BindingKind::Type:
  case BindingKind::Class:
  case BindingKind::Concept: {
    result.set_object_name("TypeBinding");
    const TypeBinding &type_binding = static_cast<const TypeBinding &>(*this);
    if (type_binding.type.has_value()) {
      result.add_object_field("type", type_binding.type.value()->serialize());
    }
  } break;
  default:
    throw RuntimeError("not implemented (serialize_binding with unknown kind)");
  }
  if (visibility != DeclarationVisibility::Default) {
    result.add_object_field("visibility", serialize_declaration_visibility(visibility));
  }
  return result;
}

} // namespace amelia
