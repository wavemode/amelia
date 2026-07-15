#pragma once

#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct AliasType : Type {
  String name;
  String module_name;
  Flex<Type> target;

  bool is_resolved() const override;
  Flex<Type> resolve() const override;

  bool is_comptime_const() const override;
  Flex<Type> remove_comptime_const() const override;

  bool unify(const Type &assignment_type) const override;
  Option<Flex<Expression>> coerce(const Type &assignment_type, const Expression &expr)
      const override;

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

  Serialize serialize() const override;
};

} // namespace amelia
