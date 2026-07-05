#pragma once

#include "util/data/set.hpp"

namespace amelia {

struct ModuleLoaderContext {
  Set<String> module_path;
};

} // namespace amelia
