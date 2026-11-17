#pragma once

#include "loader/data/abstract_loaded_module_import_item.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoadedModuleImportItem : AbstractLoadedModuleImportItem {
  String name;
  Option<String> alias;
};

} // namespace amelia
