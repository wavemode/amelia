#pragma once

#include "type/data/type.hpp"

namespace amelia {

struct ClosureType : TypeWithDynamicId<ClosureType> {
  ClosureType();
  // TODO
};

bool is_closure_type(const Type &type);

} // namespace amelia
