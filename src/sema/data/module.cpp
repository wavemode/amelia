#include "module.hpp"

#include "util/data/text_utils.hpp"

namespace amelia {

Serialize Module::serialize() const {
  auto result = Serialize();
  result.set_object_name("Module");
  result.add_object_field("name", Serialize::quoted(name));
  result.add_object_field("scope", scope->serialize());
  return result;
}

} // namespace amelia
