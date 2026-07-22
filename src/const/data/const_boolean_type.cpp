#include "const_boolean_type.hpp"

#include "builtin/data/builtin_type.hpp"
#include "operator/data/native_binary_operation.hpp"
#include "operator/data/native_unary_operation.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

ConstBooleanType::ConstBooleanType() {}

ConstBooleanType::ConstBooleanType(bool val) : value(val) {}

bool ConstBooleanType::is_comptime_const() const {
  return true;
}

Flex<Type> ConstBooleanType::internal_remove_comptime_const() const {
  return BOOL_TYPE;
}

bool ConstBooleanType::internal_unify(const Type &assignment_type) const {
  return assignment_type.is<ConstBooleanType>() &&
         value == assignment_type.as<ConstBooleanType>().value;
}

bool ConstBooleanType::has_native_numeric_repr() const {
  return true;
}

Option<Flex<Expression>> ConstBooleanType::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  if (!right_type.is<ConstBooleanType>()) {
    return None();
  }
  auto &rhs_boolean_type = right_type.as<ConstBooleanType>();
  Flex<Type> result_type;
  switch (op_kind) {
  case BinaryOperatorKind::And:
    result_type = emplace_flex<ConstBooleanType>(value && rhs_boolean_type.value);
    break;
  case BinaryOperatorKind::Or:
    result_type = emplace_flex<ConstBooleanType>(value || rhs_boolean_type.value);
    break;
  case BinaryOperatorKind::Equals:
    result_type = emplace_flex<ConstBooleanType>(value == rhs_boolean_type.value);
    break;
  case BinaryOperatorKind::NotEquals:
    result_type = emplace_flex<ConstBooleanType>(value != rhs_boolean_type.value);
    break;
  case BinaryOperatorKind::Add:
  case BinaryOperatorKind::Subtract:
  case BinaryOperatorKind::Multiply:
  case BinaryOperatorKind::Divide:
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
  case BinaryOperatorKind::Greater:
  case BinaryOperatorKind::GreaterEquals:
  case BinaryOperatorKind::Less:
  case BinaryOperatorKind::LessEquals:
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::Modulo:
  case BinaryOperatorKind::RightShift:
  case BinaryOperatorKind::Assignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::RShiftAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::AddAssignment:
    return None();
  }

  auto result_expr = emplace_flex<NativeBinaryOperationExpression>();
  result_expr->node_id = expr_node_id;
  result_expr->type = result_type;
  result_expr->op_kind = op_kind;
  result_expr->left = left_expr.flex();
  result_expr->right = right_expr.flex();
  return result_expr;
}

Option<Flex<Expression>> ConstBooleanType::perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
) const {
  Flex<Type> result_type;
  switch (op_kind) {
  case UnaryOperatorKind::Not:
    result_type = emplace_flex<ConstBooleanType>(!value);
    break;
  case UnaryOperatorKind::Negate:
  case UnaryOperatorKind::Positive:
  case UnaryOperatorKind::BitwiseNot:
  case UnaryOperatorKind::Decrement:
  case UnaryOperatorKind::Increment:
    return None();
  }
  auto result_expr = emplace_flex<NativeUnaryOperationExpression>();
  result_expr->node_id = expr_node_id;
  result_expr->type = result_type;
  result_expr->op_kind = op_kind;
  result_expr->operand = expr.flex();
  return result_expr;
}

bool ConstBooleanType::is_primitive() const {
  return true;
}

Serialize ConstBooleanType::serialize() const {
  String repr("Const[");
  repr.append(value ? Text("true") : Text("false"));
  repr.append("]");
  return Serialize::literal(move(repr));
}

} // namespace amelia
