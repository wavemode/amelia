#pragma once

#include "data/testing/pretty_print.hpp"

namespace amelia {

enum class DeclarationVisibility {
  Public,
  Private,
  Protected,
  Local,
  Default,
};

PrettyPrint pretty_print_declaration_visibility(DeclarationVisibility visibility);

} // namespace amelia
