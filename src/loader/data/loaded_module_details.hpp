#pragma once

#include "loader/data/loaded_module_import_decl.hpp"
#include "source/data/module_name.hpp"
#include "util/data/box.hpp"
#include "util/data/option.hpp"
#include "util/data/ref.hpp"

namespace amelia {

struct LoadedModuleFile;

struct LoadedModuleDetails {
  ModuleName name;
  Option<Ref<LoadedModuleFile>> file;
  Option<Ref<AbstractLoadedModuleDecl>> module_decl;
  List<Box<AbstractLoadedModuleDecl>> top_level_decls;
};

} // namespace amelia
