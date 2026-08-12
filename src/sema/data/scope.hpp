#pragma once

#include <cstddef>

#include "loader/data/source_file.hpp"
#include "sema/data/binding_ref.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct Scope {
  Flex<SourceFile> source_file;
  List<BindingRef> binding_refs;
  Map<String, size_t> binding_ids;
};

} // namespace amelia
