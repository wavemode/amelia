#pragma once

#include "util/data/set.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct ModuleLoaderContext {
  Set<String> module_path;
};

} // namespace amelia
