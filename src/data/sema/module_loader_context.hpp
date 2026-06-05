#pragma once

#include "data/util/set.hpp"

namespace amelia {

struct ModuleLoaderContext {
  Set<String> module_path;
};

} // namespace amelia
