#pragma once

#include "loader/data/abstract_loaded_module_import_item.hpp"
#include "util/data/box.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleImportDecl {
  String module;
  Option<List<Box<AbstractLoadedModuleImportItem>>> items;
  Option<String> alias;
};

} // namespace amelia
