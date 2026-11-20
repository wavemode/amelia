#pragma once

#include "loader/data/loaded_module_file.hpp"
#include "loader/data/loader_settings.hpp"
#include "util/data/box.hpp"
#include "util/data/file_path.hpp"
#include "util/data/map.hpp"

namespace amelia {

struct LoaderResult {
  Map<FilePath, Box<LoadedModuleFile>> loaded_files;
  LoaderSettings settings;
};

} // namespace amelia
