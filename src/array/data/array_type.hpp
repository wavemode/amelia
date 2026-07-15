#pragma once

#include <cstdint>

#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/utility.hpp"

namespace amelia {

struct ArrayType : TypeWithDynamicId<ArrayType> {
  Flex<Type> element_type;
  uint64_t size;

  bool is_resolved() const override;
  Flex<Type> resolve() const override;

  bool is_comptime_const() const override;
  Flex<Type> remove_comptime_const() const override;

  bool unify(const Type &assignment_type) const override;

  Option<Flex<Expression>> cast(const Type &assignment_type, const Expression &expr) const override;

  Serialize serialize() const override;
};

} // namespace amelia
