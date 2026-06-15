#include "sema_result.hpp"

namespace amelia {

void format_sema_result(AbstractString &out, const SemaResult &sema_result) {}

PrettyPrint SemaResult::pretty_print() {
  auto result = PrettyPrint();
  result.set_object_name("SemaResult");
  auto modules_list = PrettyPrint();
  for (const Module &module : modules) {
    modules_list.add_tuple_item(module.pretty_print());
  }
  result.add_object_field("modules", move(modules_list));
  return result;
}

} // namespace amelia
