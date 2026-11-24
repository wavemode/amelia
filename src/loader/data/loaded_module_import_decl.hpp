#pragma once

#include "loader/data/abstract_loaded_module_import_item.hpp"
#include "loader/data/loaded_declaration_visibility.hpp"
#include "source/data/module_name.hpp"
#include "util/data/box.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

using NodeId = int32_t;

struct LoadedModuleImportDecl : AbstractLoadedModuleDecl {
  ModuleName module;
  Option<List<Box<AbstractLoadedModuleImportItem>>> items;
  Option<String> alias;
};

} // namespace amelia
