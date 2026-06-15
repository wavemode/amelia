#include "declaration_visibility.hpp"

namespace amelia {
PrettyPrint pretty_print_declaration_visibility(DeclarationVisibility visibility) {
  String result;
  switch (visibility) {
  case DeclarationVisibility::Public:
    result.append("Public");
    break;
  case DeclarationVisibility::Private:
    result.append("Private");
    break;
  case DeclarationVisibility::Protected:
    result.append("Protected");
    break;
  case DeclarationVisibility::Local:
    result.append("Local");
    break;
  case DeclarationVisibility::Default:
    result.append("Default");
    break;
  }
  return PrettyPrint::literal(move(result));
}
} // namespace amelia
