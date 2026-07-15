#include "bitint_type.hpp"

#include "builtin/data/builtin_type.hpp"
#include "const/data/const_character_type.hpp"
#include "const/data/const_integer_type.hpp"
#include "expr/data/expression.hpp"
#include "operator/data/native_binary_operation.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

bool BitIntType::unify(const Type &assignment_type) const {
  if (is_usize_type(assignment_type)) {
    return false;
  }

  if (assignment_type.is<ConstIntegerType>()) {
    return is_signed && bit_width == assignment_type.repr_bit_size();
  }

  if (assignment_type.is<ConstCharacterType>()) {
    return bit_width == 32 && !is_signed;
  }

  return assignment_type.is_integral() && max_value() == assignment_type.max_value() &&
         min_value() == assignment_type.min_value();
}

Option<Flex<Expression>> BitIntType::coerce(const Type &assignment_type, const Expression &expr)
    const {
  if (is_usize_type(assignment_type)) {
    return None();
  }

  if (assignment_type.is_integral() &&
      can_represent_range(assignment_type.min_value(), assignment_type.max_value())) {
    return native_type_cast(*this, expr);
  }

  return None();
}

Option<Flex<Expression>> BitIntType::cast(const Type &assignment_type, const Expression &expr)
    const {
  if (assignment_type.has_native_numeric_repr()) {
    return native_type_cast(*this, expr);
  }
  return None();
}

bool BitIntType::is_integral() const {
  return true;
}

bool BitIntType::has_native_numeric_repr() const {
  return true;
}

uint64_t BitIntType::repr_bit_size() const {
  return bit_width;
}

Integer BitIntType::min_value() const {
  if (is_signed) {
    return -(Integer(1) << (bit_width - 1));
  } else {
    return Integer(0);
  }
}

Integer BitIntType::max_value() const {
  if (is_signed) {
    return (Integer(1) << (bit_width - 1)) - 1;
  } else {
    return (Integer(1) << bit_width) - 1;
  }
}

Option<Flex<Expression>> BitIntType::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  switch (op_kind) {
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::RightShift:
    return perform_native_shift(expr_node_id, op_kind, *this, left_expr, right_type, right_expr);
  case BinaryOperatorKind::Add:
  case BinaryOperatorKind::Subtract:
  case BinaryOperatorKind::Multiply:
  case BinaryOperatorKind::Divide:
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
  case BinaryOperatorKind::Modulo:
    return perform_native_binary_op(
        expr_node_id, op_kind, *this, left_expr, right_type, right_expr, *this
    );
  case BinaryOperatorKind::Equals:
  case BinaryOperatorKind::Greater:
  case BinaryOperatorKind::GreaterEquals:
  case BinaryOperatorKind::Less:
  case BinaryOperatorKind::LessEquals:
  case BinaryOperatorKind::NotEquals:
    return perform_native_binary_op(
        expr_node_id, op_kind, *this, left_expr, right_type, right_expr, BOOL_TYPE
    );
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
    return perform_native_shift(
        expr_node_id, op_kind, NULL_TYPE, left_expr, right_type, right_expr
    );
  case BinaryOperatorKind::Assignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::AddAssignment:
    return perform_native_binary_op(
        expr_node_id, op_kind, *this, left_expr, right_type, right_expr, NULL_TYPE
    );
  case BinaryOperatorKind::Or:
  case BinaryOperatorKind::And:
    return None();
  }
}

Option<Flex<Expression>> BitIntType::perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
) const {
  switch (op_kind) {
  case UnaryOperatorKind::Negate:
    if (!is_signed) {
      return None();
    }
    [[fallthrough]];
  case UnaryOperatorKind::Positive:
  case UnaryOperatorKind::BitwiseNot:
    return perform_native_unary_op(expr_node_id, op_kind, *this, expr);
  case UnaryOperatorKind::Decrement:
  case UnaryOperatorKind::Increment:
    return perform_native_unary_op(expr_node_id, op_kind, NULL_TYPE, expr);
  case UnaryOperatorKind::Not:
    return None();
  }
}

bool BitIntType::is_primitive() const {
  return bit_width <= 64;
}

Serialize BitIntType::serialize() const {
  String repr;
  if (!is_signed) {
    repr.append('u');
  }
  repr.append("bitint[");
  TextUtils::to_string(repr, bit_width);
  repr.append(']');
  return Serialize::literal(repr);
}

} // namespace amelia
