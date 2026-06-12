#include "module.hpp"

#include "data/util/text_utils.hpp"

namespace amelia {

void format_module(AbstractString &out, const Module &module) {}

PrettyPrint Module::pretty_print() const {
  auto result = PrettyPrintObject();
  result.set_name("Module");
  result.add_field("name", PrettyPrintString::quoted(name));
  result.add_field("scope", scope->pretty_print());
  return result;
}

} // namespace amelia
