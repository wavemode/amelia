#pragma once

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleImportDeclItem : AbstractLoadedModuleDecl {
  String module;
  Option<String> alias;
};

} // namespace amelia
