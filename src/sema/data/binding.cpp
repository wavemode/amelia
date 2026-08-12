#include "binding.hpp"

#include "util/data/serialize.hpp"

namespace amelia {

Serialize Binding::serialize() const {
  auto result = Serialize();
  result.add_object_field("name", Serialize::quoted(name));
  result.add_object_field("kind", serialize_binding_kind(kind));
  result.add_object_field("type", type->serialize());
  if (visibility != DeclarationVisibility::Default) {
    result.add_object_field("visibility", serialize_declaration_visibility(visibility));
  }
  return result;
}

} // namespace amelia
