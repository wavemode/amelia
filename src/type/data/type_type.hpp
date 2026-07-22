#pragma once

#include "type/data/type.hpp"

namespace amelia {

struct TypeType : Type {
  Flex<Type> referenced_type;
};

} // namespace amelia
