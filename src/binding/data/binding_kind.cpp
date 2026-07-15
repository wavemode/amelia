#include "binding_kind.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

Serialize serialize_binding_kind(BindingKind kind) {
  String result;
  switch (kind) {
  case BindingKind::Variable:
    result.append("Variable");
    break;
  case BindingKind::Constant:
    result.append("Constant");
    break;
  case BindingKind::Function:
    result.append("Function");
    break;
  case BindingKind::Type:
    result.append("Type");
    break;
  case BindingKind::Class:
    result.append("Class");
    break;
  case BindingKind::Concept:
    result.append("Concept");
    break;
  case BindingKind::Module:
    result.append("Module");
    break;
  }
  return Serialize::literal(move(result));
}

} // namespace amelia
