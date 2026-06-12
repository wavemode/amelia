#include "scope.hpp"

namespace amelia {

PrettyPrint Scope::pretty_print() const {
  auto result = PrettyPrintObject();
  result.set_name("Scope");
  auto bindings_list = PrettyPrintList();
  for (const auto &[binding_name, binding_id] : binding_ids) {
    const Binding &binding = *bindings[binding_id];
    bindings_list.add_item(binding.pretty_print(String(binding_name)));
  }
  result.add_field("bindings", move(bindings_list));
  return result;
}

} // namespace amelia
