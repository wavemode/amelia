#pragma once

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleImportDeclModule : AbstractLoadedModuleDecl {
  String module;
  Option<String> alias;
};

} // namespace amelia
