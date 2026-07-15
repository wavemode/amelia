#pragma once

#include "type/data/type.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct ImplType : Type {
  ImplType();
  Flex<Type> concept_type;
};

bool is_impl_type(const Type &type);

} // namespace amelia
