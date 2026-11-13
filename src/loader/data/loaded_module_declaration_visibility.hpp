#pragma once

#include "source/data/declaration_visibility.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleDeclarationVisibility {
  DeclarationVisibility visibility;
  Option<String> scope;
};

} // namespace amelia
