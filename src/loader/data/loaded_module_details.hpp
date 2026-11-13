#pragma once

#include <cstdint>

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "loader/data/loaded_module_import_decl_item.hpp"
#include "util/data/box.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleFile;
using NodeId = int32_t;

struct LoadedModuleDetails {
  LoadedModuleDetails(String name, LoadedModuleFile &module_file)
      : name(move(name)), module_file(module_file) {}

  String name;
  LoadedModuleDeclarationVisibility visibility;
  Option<Ref<LoadedModuleDetails>> parent_module;
  Ref<LoadedModuleFile> module_file;
  Option<NodeId> name_decl;
  Map<String, Box<AbstractLoadedModuleDecl>> decls;
};

} // namespace amelia
