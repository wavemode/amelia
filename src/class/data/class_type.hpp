#pragma once

#include "type/data/type.hpp"

namespace amelia {

struct ClassType : TypeWithDynamicId<ClassType> {
  ClassType();
  // TODO
};

bool is_class_type(const Type &type);

} // namespace amelia
