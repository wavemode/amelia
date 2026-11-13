#pragma once

#include <cstdint>

namespace amelia {

class Serialize;

enum class DeclarationVisibility : uint8_t {
  Public,
  Private,
  Protected,
  Local,
  Default,
};

Serialize serialize_declaration_visibility(DeclarationVisibility visibility);

} // namespace amelia
