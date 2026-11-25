#pragma once

#include "loader/data/loaded_module_file.hpp"
#include "source/data/module_name.hpp"
#include "util/data/box.hpp"
#include "util/data/file_path.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/ref.hpp"

namespace amelia {

struct LoaderResult {
  Map<FilePath, Box<LoadedModuleFile>> loaded_files;
  Map<ModuleName, List<Ref<LoadedModuleDetails>>> loaded_modules;
};

} // namespace amelia
