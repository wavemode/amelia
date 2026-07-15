#pragma once

#include "type/data/type.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct SliceType : TypeWithDynamicId<SliceType> {
  Flex<Type> element_type;

  bool is_resolved() const override;
  Flex<Type> resolve() const override;

  bool is_comptime_const() const override;
  Flex<Type> remove_comptime_const() const override;

  bool unify(const Type &assignment_type) const override;

  Serialize serialize() const override;
};

} // namespace amelia
