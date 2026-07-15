#pragma once

namespace amelia {

class Serialize;

enum class DeclarationVisibility {
  Public,
  Private,
  Protected,
  Local,
  Default,
};

Serialize serialize_declaration_visibility(DeclarationVisibility visibility);

} // namespace amelia
