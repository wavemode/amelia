#pragma once

#include "type/data/type.hpp"

namespace amelia {

struct ClassType : Type {
  ClassType();
  // TODO
};

bool is_class_type(const Type &type);

} // namespace amelia
