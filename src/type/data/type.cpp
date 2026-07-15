#include "type.hpp"

#include "expr/data/expression.hpp"
#include "util/data/integer.hpp"
#include "util/data/option.hpp"

namespace amelia {

bool Type::is_resolved() const {
  return true;
}

Flex<Type> Type::resolve() const {
  return flex();
}

Flex<Type> Type::resolve_if_needed() const {
  if (is_resolved()) {
    return flex();
  }
  return resolve();
}

bool Type::is_comptime_const() const {
  return false;
}

Flex<Type> Type::remove_comptime_const() const {
  return flex();
}

Flex<Type> Type::remove_comptime_const_if_needed() const {
  if (is_comptime_const()) {
    return remove_comptime_const();
  }
  return flex();
}

bool Type::unify(const Type &assignment_type) const {
  return this == &assignment_type;
}

Option<Flex<Expression>> Type::coerce(const Expression &expr) const {
  return coerce(expr.type, expr);
}

Option<Flex<Expression>> Type::coerce(const Type &assignment_type, const Expression &expr) const {
  if (unify(assignment_type)) {
    return expr.flex();
  }
  return None();
}

Option<Flex<Expression>> Type::cast(const Expression &expr) const {
  return cast(expr.type, expr);
}

Option<Flex<Expression>> Type::cast(const Type &assignment_type, const Expression &expr) const {
  if (unify(assignment_type)) {
    return expr.flex();
  }
  return None();
}

bool Type::is_floating_point() const {
  return false;
}

bool Type::is_integral() const {
  return false;
}

bool Type::has_native_numeric_repr() const {
  return false;
}

uint64_t Type::repr_bit_size() const {
  throw RuntimeError(
      "Type::repr_bit_size() called on a type that does not have a native numeric representation"
  );
}

Integer Type::min_value() const {
  throw RuntimeError("Type::min_value() called on a type that is not integral");
}

Integer Type::max_value() const {
  throw RuntimeError("Type::max_value() called on a type that is not integral");
}

bool Type::can_represent_range(const Integer &min, const Integer &max) const {
  if (!is_integral()) {
    return false;
  }
  return min >= min_value() && max <= max_value();
}

Option<Flex<Expression>> Type::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  return None();
}

Option<Flex<Expression>> Type::perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
) const {
  return None();
}

bool Type::is_trivial() const {
  return true;
}

bool Type::is_primitive() const {
  return false;
}

} // namespace amelia
