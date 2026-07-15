#include "const_rational_type.hpp"

#include "builtin/data/builtin_type.hpp"
#include "const/data/const_boolean_type.hpp"
#include "const/data/const_character_type.hpp"
#include "const/data/const_integer_type.hpp"
#include "operator/data/native_binary_operation.hpp"
#include "operator/data/native_unary_operation.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

ConstRationalType::ConstRationalType() {}

ConstRationalType::ConstRationalType(Rational &&val) : value(move(val)) {}

bool ConstRationalType::is_comptime_const() const {
  return true;
}

Flex<Type> ConstRationalType::remove_comptime_const() const {
  return DOUBLE_TYPE;
}

bool ConstRationalType::unify(const Type &assignment_type) const {
  return assignment_type.is<ConstRationalType>() &&
         value == assignment_type.as<ConstRationalType>().value;
}

bool ConstRationalType::is_floating_point() const {
  return true;
}

bool ConstRationalType::has_native_numeric_repr() const {
  return true;
}

Option<Flex<Expression>> ConstRationalType::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  Rational rhs_value;
  if (right_type.is<ConstRationalType>()) {
    rhs_value = right_type.as<ConstRationalType>().value;
  } else if (right_type.is<ConstIntegerType>()) {
    rhs_value = Rational(right_type.as<ConstIntegerType>().value);
  } else if (right_type.is<ConstCharacterType>()) {
    rhs_value = Rational(right_type.as<ConstCharacterType>().value);
  } else {
    return None();
  }

  Flex<Type> result_type;
  switch (op_kind) {
  case BinaryOperatorKind::Add:
    result_type = emplace_flex<ConstRationalType>(value + rhs_value);
    break;
  case BinaryOperatorKind::Subtract:
    result_type = emplace_flex<ConstRationalType>(value - rhs_value);
    break;
  case BinaryOperatorKind::Multiply:
    result_type = emplace_flex<ConstRationalType>(value * rhs_value);
    break;
  case BinaryOperatorKind::Divide:
    result_type = emplace_flex<ConstRationalType>(value / rhs_value);
    break;
  case BinaryOperatorKind::Equals:
    result_type = emplace_flex<ConstBooleanType>(value == rhs_value);
    break;
  case BinaryOperatorKind::Greater:
    result_type = emplace_flex<ConstBooleanType>(value > rhs_value);
    break;
  case BinaryOperatorKind::GreaterEquals:
    result_type = emplace_flex<ConstBooleanType>(value >= rhs_value);
    break;
  case BinaryOperatorKind::Less:
    result_type = emplace_flex<ConstBooleanType>(value < rhs_value);
    break;
  case BinaryOperatorKind::LessEquals:
    result_type = emplace_flex<ConstBooleanType>(value <= rhs_value);
    break;
  case BinaryOperatorKind::NotEquals:
    result_type = emplace_flex<ConstBooleanType>(value != rhs_value);
    break;
  case BinaryOperatorKind::And:
  case BinaryOperatorKind::Or:
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
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

Option<Flex<Expression>> ConstRationalType::perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
) const {
  auto result_type = emplace_flex<ConstRationalType>();
  switch (op_kind) {
  case UnaryOperatorKind::Negate:
    result_type->value = -value;
    break;
  case UnaryOperatorKind::Positive:
    result_type->value = value;
    break;
  case UnaryOperatorKind::Not:
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

Serialize ConstRationalType::serialize() const {
  String repr("Const[");
  value.to_decimal_string(repr, 12);
  repr.append(']');
  return Serialize::literal(repr);
}

} // namespace amelia
