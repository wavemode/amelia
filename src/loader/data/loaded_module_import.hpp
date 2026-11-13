#pragma once

#include "loaded_module_declaration_visibility.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleImport {
  String module;
  String name;
  Option<String> alias;
  LoadedModuleDeclarationVisibility visibility;
};

} // namespace amelia
