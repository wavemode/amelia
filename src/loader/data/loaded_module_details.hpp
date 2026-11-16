#pragma once

#include <cstdint>

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "loader/data/loaded_module_import_decl_item.hpp"
#include "util/data/box.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleFile;
using NodeId = int32_t;

struct LoadedModuleDetails {
  LoadedModuleDetails(LoadedModuleFile &module_file)
      : module_file(module_file), imports() {}

  String name;
  NodeId root_decl = -1;
  LoadedModuleDeclarationVisibility visibility;
  Option<Ref<LoadedModuleDetails>> parent_module;
  Ref<LoadedModuleFile> module_file;
  List<Box<AbstractLoadedModuleDecl>> imports;
  Map<String, Box<AbstractLoadedModuleDecl>> decls;
};

} // namespace amelia
