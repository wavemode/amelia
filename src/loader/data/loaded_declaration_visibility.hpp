#pragma once

#include "source/data/declaration_visibility.hpp"
#include "util/data/option.hpp"
#include "source/data/module_name.hpp"

namespace amelia {

struct ModuleName;

struct LoadedDeclarationVisibility {
  DeclarationVisibility visibility = DeclarationVisibility::Default;
  Option<ModuleName> scope;
};

} // namespace amelia
