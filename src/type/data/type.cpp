#include "type.hpp"

#include "builtin/data/builtin_type.hpp"
#include "expr/data/expression.hpp"
#include "util/data/integer.hpp"
#include "util/data/option.hpp"

namespace amelia {

Flex<Type> Type::resolve() const {
  if (is_resolved()) {
    return flex();
  }
  return internal_resolve();
}

Flex<Type> Type::remove_comptime_const() const {
  if (!is_comptime_const()) {
    return flex();
  }
  return internal_remove_comptime_const();
}

bool Type::unify(const Type &type) const {
  const Type &assignment_type = type.resolve();

  if (this == &assignment_type) {
    return true;
  }

  if (is_never_type(assignment_type)) {
    return true;
  }

  if (is_unknown_type(*this) || is_unknown_type(assignment_type)) {
    return false;
  }

  return internal_unify(assignment_type);
}

Option<Flex<Expression>> Type::coerce(const Expression &expr) const {
  return coerce(expr.type, expr);
}

Option<Flex<Expression>> Type::coerce(const Type &type, const Expression &expr) const {
  const Type &expr_type = type.resolve();

  if (unify(expr_type)) {
    return expr.flex();
  }

  return internal_coerce(expr_type, expr);
}

Option<Flex<Expression>> Type::cast(const Expression &expr) const {
  return cast(expr.type, expr);
}

Option<Flex<Expression>> Type::cast(const Type &type, const Expression &expr) const {
  const Type &expr_type = type.resolve();

  if (unify(expr_type)) {
    return expr.flex();
  }

  return internal_cast(expr_type, expr);
}

bool Type::is_resolved() const {
  return true;
}

Flex<Type> Type::internal_resolve() const {
  return flex();
}

bool Type::is_comptime_const() const {
  return false;
}

Flex<Type> Type::internal_remove_comptime_const() const {
  return flex();
}

bool Type::internal_unify(const Type &assignment_type) const {
  return this == &assignment_type;
}

Option<Flex<Expression>> Type::internal_coerce(const Type &, const Expression &) const {
  return None();
}

Option<Flex<Expression>> Type::internal_cast(const Type &, const Expression &) const {
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

Option<Flex<Expression>> Type::
    perform_binary_op(NodeId, BinaryOperatorKind, const Expression &, const Type &, const Expression &)
        const {
  return None();
}

Option<Flex<Expression>> Type::perform_unary_op(NodeId, UnaryOperatorKind, const Expression &)
    const {
  return None();
}

bool Type::is_trivial() const {
  return true;
}

bool Type::is_primitive() const {
  return false;
}

} // namespace amelia
