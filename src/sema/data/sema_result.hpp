#pragma once

#include "module/data/module_type.hpp"
#include "util/data/deque.hpp"
#include "util/data/flex.hpp"
#include "util/data/map.hpp"

namespace amelia {

class Serialize;

struct SemaResult {
  Map<String, Flex<ModuleType>> modules;

  Serialize serialize();
};

} // namespace amelia
