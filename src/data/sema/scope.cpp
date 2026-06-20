#include "scope.hpp"

namespace amelia {

Serialize Scope::serialize() const {
  auto result = Serialize();
  result.set_object_name("Scope");
  if (active_bindings.size() > 0) {
    List<Text> binding_names;
    for (Text binding_name : active_binding_ids.keys()) {
      binding_names.push_back(binding_name);
    }
    binding_names.sort();

    auto bindings_list = Serialize();
    for (Text binding_name : binding_names) {
      const BindingId &binding_id = active_binding_ids.get(binding_name);
      bindings_list.add_list_item(active_bindings[binding_id]->serialize());
    }
    result.add_object_field("bindings", move(bindings_list));
  }
  return result;
}

} // namespace amelia
