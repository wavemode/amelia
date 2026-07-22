#pragma once

#include <cstdint>

#include "type/data/type.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct ConstStringType : Type {
  ConstStringType();
  ConstStringType(String &&val);
  String value;

  bool is_comptime_const() const override;
  Flex<Type> internal_remove_comptime_const() const override;

  bool internal_unify(const Type &assignment_type) const override;

  Option<Flex<Expression>> perform_binary_op(
      NodeId expr_node_id,
      BinaryOperatorKind op_kind,
      const Expression &left_expr,
      const Type &right_type,
      const Expression &right_expr
  ) const override;

  Serialize serialize() const override;
};

} // namespace amelia
