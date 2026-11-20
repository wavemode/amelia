#pragma once

#include <cstdint>

#include "loader/data/loaded_declaration_visibility.hpp"
#include "loader/data/loaded_module_import_decl.hpp"
#include "loader/data/loaded_module_item_decl.hpp"
#include "source/data/module_name.hpp"
#include "util/data/box.hpp"
#include "util/data/map.hpp"
#include "util/data/option.hpp"
#include "util/data/ref.hpp"
#include "util/data/text.hpp"

namespace amelia {

struct LoadedModuleFile;
struct LoadedModuleSubmoduleDecl;
using NodeId = int32_t;

struct LoadedModuleDetails {
  ModuleName name;
  String short_name;
  List<NodeId> annotations;
  Option<Ref<LoadedModuleFile>> file;
  LoadedDeclarationVisibility visibility;
  List<Box<LoadedModuleImportDecl>> imports;
  List<Box<LoadedModuleItemDecl>> items;
  Map<Text, Box<LoadedModuleSubmoduleDecl>> submodules;
  Option<Ref<LoadedModuleDetails>> parent;

  ~LoadedModuleDetails();
};

} // namespace amelia
