#pragma once

#include "util/data/set.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoaderContext {
  Set<String> module_path;
};

} // namespace amelia
