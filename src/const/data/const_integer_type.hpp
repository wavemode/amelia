#pragma once

#include "type/data/type.hpp"
#include "util/data/integer.hpp"

namespace amelia {

struct ConstIntegerType : Type {
  ConstIntegerType();
  ConstIntegerType(Integer &&val);
  Integer value;

  bool is_comptime_const() const override;
  Flex<Type> remove_comptime_const() const override;

  bool unify(const Type &assignment_type) const override;

  bool is_integral() const override;
  bool has_native_numeric_repr() const override;
  uint64_t repr_bit_size() const override;
  Integer min_value() const override;
  Integer max_value() const override;

  Option<Flex<Expression>> perform_binary_op(
      NodeId expr_node_id,
      BinaryOperatorKind op_kind,
      const Expression &left_expr,
      const Type &right_type,
      const Expression &right_expr
  ) const override;

  Option<Flex<Expression>> perform_unary_op(
      NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
  ) const override;

  bool is_primitive() const override;

  Serialize serialize() const override;
};

} // namespace amelia
