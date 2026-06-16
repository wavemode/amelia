#include "scope.hpp"

namespace amelia {

Serialize Scope::serialize() const {
  auto result = Serialize();
  result.set_object_name("Scope");
  if (bindings.size() > 0) {
    auto bindings_list = Serialize();
    for (const auto &binding : bindings) {
      bindings_list.add_list_item(binding->serialize());
    }
    result.add_object_field("bindings", move(bindings_list));
  }
  return result;
}

} // namespace amelia
