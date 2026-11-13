#pragma once

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "loader/data/loaded_module_details.hpp"

namespace amelia {

struct LoadedModuleSubmoduleDecl : AbstractLoadedModuleDecl {
  LoadedModuleSubmoduleDecl(String name, LoadedModuleFile &module_file)
      : details(move(name), module_file) {}

  LoadedModuleDetails details;
};

} // namespace amelia
