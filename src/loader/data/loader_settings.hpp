#pragma once

#include "util/data/file_path.hpp"
#include "util/data/list.hpp"

namespace amelia {

struct LoaderSettings {
  List<FilePath> search_path;
};

} // namespace amelia
