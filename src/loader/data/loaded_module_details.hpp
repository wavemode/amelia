#pragma once

#include "loader/data/loaded_module_declaration.hpp"
#include "loader/data/loaded_module_import.hpp"
#include "util/data/box.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleFile;

struct LoadedModuleDetails {
  String name;
  Ref<LoadedModuleFile> module_file;
  Map<String, Box<LoadedModuleImport>> imports;
  Map<String, Box<LoadedModuleDeclaration>> decls;
  Map<String, Box<LoadedModuleDetails>> submodules;
};

} // namespace amelia
