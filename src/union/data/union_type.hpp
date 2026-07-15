#pragma once

#include "type/data/type.hpp"

namespace amelia {

struct UnionType : TypeWithDynamicId<UnionType> {
  UnionType();
  // TODO
};

bool is_union_type(const Type &type);

} // namespace amelia
