#pragma once

#include <cstdint>

#include "loader/data/abstract_loaded_module_decl.hpp"
#include "loader/data/loaded_module_import_decl.hpp"
#include "loader/data/loaded_module_import_decl_item.hpp"
#include "util/data/box.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleFile;
using NodeId = int32_t;

struct LoadedModuleDetails {
  String name;
  Option<NodeId> root_decl_node_id;
  Option<NodeId> name_decl_node_id;
  LoadedModuleDeclarationVisibility visibility;
  Option<Ref<LoadedModuleDetails>> parent_module;
  Option<Ref<LoadedModuleFile>> module_file;
  List<Box<LoadedModuleImportDecl>> imports;
  Map<String, Box<AbstractLoadedModuleDecl>> decls;
};

} // namespace amelia
