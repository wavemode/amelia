#include "alias_type.hpp"

#include "expr/data/expression.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"

namespace amelia {
bool AliasType::is_resolved() const {
  return false;
}

Flex<Type> AliasType::internal_resolve() const {
  return target;
}

bool AliasType::is_comptime_const() const {
  return target->is_comptime_const();
}

Flex<Type> AliasType::internal_remove_comptime_const() const {
  return target->remove_comptime_const();
}

bool AliasType::internal_unify(const Type &assignment_type) const {
  return target->unify(assignment_type);
}

Option<Flex<Expression>> AliasType::internal_coerce(
    const Type &assignment_type, const Expression &expr
) const {
  if (unify(assignment_type)) {
    return expr.flex();
  }
  return target->coerce(assignment_type, expr);
}

Option<Flex<Expression>> AliasType::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  auto result = target->perform_binary_op(expr_node_id, op_kind, left_expr, right_type, right_expr);
  if (result.has_value() && unify(result.value()->type)) {
    result.value()->type = flex();
  }
  return result;
}

Option<Flex<Expression>> AliasType::perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
) const {
  auto result = target->perform_unary_op(expr_node_id, op_kind, expr);
  if (result.has_value() && unify(result.value()->type)) {
    result.value()->type = flex();
  }
  return result;
}

Serialize AliasType::serialize() const {
  String repr(module_name);
  repr.append("::");
  repr.append(name);
  repr.append(" (aka ");
  target->serialize().to_string(repr);
  repr.append(')');
  return Serialize::literal(repr);
}

} // namespace amelia
