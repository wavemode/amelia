#include "implicit_value_binding_statement.hpp"

#include "sema/data/binding.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize ImplicitValueBindingStatement::serialize() const {
  Serialize result;
  result.set_object_name("ImplicitValueBindingStatement");
  Serialize bindings_list;
  for (const auto &binding : bindings) {
    bindings_list.add_list_item(binding->serialize());
  }
  result.add_object_field("bindings", bindings_list);
  result.add_object_field("body", body->serialize());
  return result;
}

} // namespace amelia
