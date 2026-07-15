#pragma once

#include "type/data/type.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct ReferenceType : TypeWithDynamicId<ReferenceType> {
  Flex<Type> referent;
  bool is_const;
  bool is_move;

  bool is_resolved() const override;
  Flex<Type> resolve() const override;

  bool is_comptime_const() const override;
  Flex<Type> remove_comptime_const() const override;

  bool unify(const Type &assignment_type) const override;
  Option<Flex<Expression>> coerce(const Type &assignment_type, const Expression &expr)
      const override;

  Serialize serialize() const override;
};

} // namespace amelia
