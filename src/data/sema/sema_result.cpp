#include "sema_result.hpp"

namespace amelia {

void format_sema_result(AbstractString &out, const SemaResult &sema_result) {}

PrettyPrint SemaResult::pretty_print() {
  auto result = PrettyPrintObject();
  result.set_name("SemaResult");
  auto modules_list = PrettyPrintList();
  for (const Module &module : modules) {
    modules_list.add_item(module.pretty_print());
  }
  result.add_field("modules", move(modules_list));
  return result;
}

} // namespace amelia
