#pragma once

#include "prelude.hpp"

namespace amelia {

enum class DeclarationVisibility {
  Public,
  Private,
  Protected,
  Local,
  Default,
};

Serialize serialize_declaration_visibility(DeclarationVisibility visibility);

} // namespace amelia
