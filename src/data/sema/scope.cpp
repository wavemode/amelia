#include "scope.hpp"

namespace amelia {

Serialize Scope::serialize() const {
  auto result = Serialize();
  result.set_object_name("Scope");
  if (bindings.size() > 0) {
    auto bindings_list = Serialize();
    for (const auto &[binding_name, binding_id] : binding_ids) {
      const Binding &binding = *bindings[binding_id];
      bindings_list.add_list_item(binding.serialize(String(binding_name)));
    }
    result.add_object_field("bindings", move(bindings_list));
  }
  return result;
}

} // namespace amelia
