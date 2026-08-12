#pragma once

#include <cstddef>

#include "loader/data/source_file.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct LoaderResult {
  List<Flex<SourceFile>> loaded_files;
  Map<String, size_t> file_id_by_module_name;

  const Flex<SourceFile> &get_source_file_by_module_name(const String &module_name) const {
    size_t file_id = file_id_by_module_name.get(module_name);
    return loaded_files[file_id];
  }
};

} // namespace amelia
