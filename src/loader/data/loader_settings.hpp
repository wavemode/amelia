#pragma once

#include "util/data/list.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoaderSettings {
  List<String> search_path;
};

} // namespace amelia
