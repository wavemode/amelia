#include "sema_result.hpp"

#include "testing/data/serialize.hpp"

namespace amelia {

Serialize SemaResult::serialize() {
  auto result = Serialize();
  result.set_object_name("SemaResult");
  auto modules_list = Serialize();
  for (const Module &module : modules) {
    modules_list.add_list_item(module.serialize());
  }
  result.add_object_field("modules", move(modules_list));
  return result;
}

} // namespace amelia
