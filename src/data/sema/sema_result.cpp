#include "sema_result.hpp"

namespace amelia {

void format_sema_result(AbstractString &out, const SemaResult &sema_result) {}

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
