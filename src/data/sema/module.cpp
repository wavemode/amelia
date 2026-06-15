#include "module.hpp"

#include "data/util/text_utils.hpp"

namespace amelia {

void format_module(AbstractString &out, const Module &module) {}

PrettyPrint Module::pretty_print() const {
  auto result = PrettyPrint();
  result.set_object_name("Module");
  result.add_object_field("name", PrettyPrint::quoted(name));
  result.add_object_field("scope", scope->pretty_print());
  return result;
}

} // namespace amelia
