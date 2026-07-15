#pragma once

#include <cstdint>

#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct StructType : TypeWithDynamicId<StructType> {
  StructType();
  Map<String, Flex<Type>> fields;
};

} // namespace amelia
