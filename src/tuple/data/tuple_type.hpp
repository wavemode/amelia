#pragma once

#include <cstdint>

#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct Expression;

struct TupleType : Type {
  List<Flex<Type>> element_types;

  virtual bool is_resolved() const override;
  virtual Flex<Type> internal_resolve() const override;

  virtual bool is_comptime_const() const override;
  virtual Flex<Type> internal_remove_comptime_const() const override;

  virtual bool internal_unify(const Type &assignment_type) const override;
  virtual Option<Flex<Expression>> internal_coerce(
      const Type &assignment_type, const Expression &expr
  ) const override;

  Serialize serialize() const override;

private:
  mutable Option<bool> is_resolved_cached;
  mutable Option<Flex<Type>> resolved_type_cached;
  mutable Option<bool> has_comptime_const_cached;
  mutable Option<Flex<Type>> remove_comptime_const_cached;
};

} // namespace amelia
