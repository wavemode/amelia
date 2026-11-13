#pragma once

#include "loaded_module_file.hpp"
#include "util/data/box.hpp"
#include "util/data/map.hpp"
#include "util/data/ref.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoaderResult {
  Map<String, Box<LoadedModuleFile>> module_files; // indexed by file path
  Map<String, Ref<LoadedModuleDetails>>
      imported_modules; // indexed by module name
};

} // namespace amelia
