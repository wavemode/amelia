#pragma once

#include "type/data/type.hpp"

namespace amelia {

struct TypeType : Type {
  Flex<Type> referenced_type;

  bool internal_unify(const Type &assignment_type) const override;

  Serialize serialize() const override;
};

} // namespace amelia
