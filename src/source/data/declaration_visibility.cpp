#include "declaration_visibility.hpp"

#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {
Serialize serialize_declaration_visibility(DeclarationVisibility visibility) {
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
  return Serialize::literal(move(result));
}
} // namespace amelia
