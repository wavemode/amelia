#pragma once

#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/map.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct ClassType : Type {
  String name;
  String module_name;
  Map<String, Flex<Type>> fields;
  Map<String, Flex<Type>> static_fields;
  Map<String, Flex<Type>> methods;
  bool is_record;
};

} // namespace amelia
