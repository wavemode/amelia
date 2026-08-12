#pragma once

#include "source/data/location.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct SourceFileImport {
  String name;
  Location location;
};

} // namespace amelia
