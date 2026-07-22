#include <climits>

#include "const_integer_type.hpp"

#include "bitint/data/bitint_type.hpp"
#include "builtin/data/builtin_type.hpp"
#include "const/data/const_boolean_type.hpp"
#include "const/data/const_character_type.hpp"
#include "const/data/const_rational_type.hpp"
#include "operator/data/native_binary_operation.hpp"
#include "operator/data/native_unary_operation.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/char_iterator.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {
ConstIntegerType::ConstIntegerType() {}

ConstIntegerType::ConstIntegerType(Integer &&val) : value(move(val)) {}

bool ConstIntegerType::is_comptime_const() const {
  return true;
}

Flex<Type> ConstIntegerType::internal_remove_comptime_const() const {
  auto bit_width = repr_bit_size();
  if (bit_width == 32) {
    return INT_TYPE;
  } else if (bit_width == 64) {
    return LONG_TYPE;
  } else {
    auto result = emplace_flex<BitIntType>();
    result->bit_width = bit_width;
    result->is_signed = true;
    return result;
  }
}

bool ConstIntegerType::internal_unify(const Type &assignment_type) const {
  return assignment_type.is<ConstIntegerType>() &&
         value == assignment_type.as<ConstIntegerType>().value;
}

bool ConstIntegerType::is_integral() const {
  return true;
}

bool ConstIntegerType::has_native_numeric_repr() const {
  return true;
}

uint64_t ConstIntegerType::repr_bit_size() const {
  // ConstIntegers with values within the int32_t range are represented as int32_t, and within
  // the int64_t rande are represented as int64_t. Any larger than that, and we use the smallest
  // signed power-of-2 bitint which can represent the value.
  if (value >= INT32_MIN && value <= INT32_MAX) {
    return 32;
  } else if (value >= INT64_MIN && value <= INT64_MAX) {
    return 64;
  }

  uint32_t bit_width = 1;
  Integer abs_value = value.abs();
  while (abs_value > 0) {
    abs_value /= 2;
    ++bit_width;
  }

  uint32_t next_power_of_two = 1;
  while (next_power_of_two < bit_width) {
    next_power_of_two <<= 1;
  }
  return next_power_of_two;
}

Integer ConstIntegerType::min_value() const {
  return value;
}

Integer ConstIntegerType::max_value() const {
  return value;
}

Option<Flex<Expression>> ConstIntegerType::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  Rational rhs_value;
  bool rhs_was_char = false;
  bool rhs_was_rational = false;
  if (right_type.is<ConstIntegerType>()) {
    rhs_value = right_type.as<ConstIntegerType>().value;
  } else if (right_type.is<ConstCharacterType>()) {
    rhs_value = Rational(right_type.as<ConstCharacterType>().value);
    rhs_was_char = true;
  } else if (right_type.is<ConstRationalType>()) {
    rhs_value = right_type.as<ConstRationalType>().value;
    rhs_was_rational = true;
  } else {
    return None();
  }

  Flex<Type> result_type;
  switch (op_kind) {
  case BinaryOperatorKind::Add:
    if (rhs_was_rational) {
      result_type = emplace_flex<ConstRationalType>(Rational(value) + rhs_value);
    } else {
      result_type = emplace_flex<ConstIntegerType>(value + rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::Subtract:
    if (rhs_was_rational) {
      result_type = emplace_flex<ConstRationalType>(Rational(value) - rhs_value);
    } else {
      result_type = emplace_flex<ConstIntegerType>(value - rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::Multiply:
    if (rhs_was_rational) {
      result_type = emplace_flex<ConstRationalType>(Rational(value) * rhs_value);
    } else {
      result_type = emplace_flex<ConstIntegerType>(value * rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::Divide:
    if (rhs_was_rational) {
      result_type = emplace_flex<ConstRationalType>(Rational(value) / rhs_value);
    } else {
      result_type = emplace_flex<ConstIntegerType>(value / rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::BitwiseAnd:
    if (rhs_was_rational) {
      return None();
    } else {
      result_type = emplace_flex<ConstIntegerType>(value & rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::BitwiseOr:
    if (rhs_was_rational) {
      return None();
    } else {
      result_type = emplace_flex<ConstIntegerType>(value | rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::BitwiseXor:
    if (rhs_was_rational) {
      return None();
    } else {
      result_type = emplace_flex<ConstIntegerType>(value ^ rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::Modulo:
    if (rhs_was_rational) {
      return None();
    } else {
      result_type = emplace_flex<ConstIntegerType>(value % rhs_value.numerator());
    }
    break;
  case BinaryOperatorKind::LeftShift:
    if (rhs_was_rational || rhs_value < 0 || rhs_value > UINT32_MAX) {
      return None();
    }
    result_type = emplace_flex<ConstIntegerType>(value << rhs_value.to_uint32());
    break;
  case BinaryOperatorKind::RightShift:
    if (rhs_was_rational || rhs_value < 0 || rhs_value > UINT32_MAX) {
      return None();
    }
    result_type = emplace_flex<ConstIntegerType>(value >> rhs_value.to_uint32());
    break;
  case BinaryOperatorKind::Equals:
    result_type = emplace_flex<ConstBooleanType>(Rational(value) == rhs_value);
    break;
  case BinaryOperatorKind::Greater:
    result_type = emplace_flex<ConstBooleanType>(Rational(value) > rhs_value);
    break;
  case BinaryOperatorKind::GreaterEquals:
    result_type = emplace_flex<ConstBooleanType>(Rational(value) >= rhs_value);
    break;
  case BinaryOperatorKind::Less:
    result_type = emplace_flex<ConstBooleanType>(Rational(value) < rhs_value);
    break;
  case BinaryOperatorKind::LessEquals:
    result_type = emplace_flex<ConstBooleanType>(Rational(value) <= rhs_value);
    break;
  case BinaryOperatorKind::NotEquals:
    result_type = emplace_flex<ConstBooleanType>(Rational(value) != rhs_value);
    break;
  case BinaryOperatorKind::And:
  case BinaryOperatorKind::Or:
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

  if (rhs_was_char && result_type->is<ConstIntegerType>()) {
    auto &const_int_type = result_type->as<ConstIntegerType>();
    if (const_int_type.value >= 0 && const_int_type.value <= UINT32_MAX) {
      uint32_t char_value = const_int_type.value.to_uint32();
      if (CharIterator::is_valid_code_point(char_value)) {
        result_type = emplace_flex<ConstCharacterType>(char_value);
      }
    }
  }

  auto result_expr = emplace_flex<NativeBinaryOperationExpression>();
  result_expr->node_id = expr_node_id;
  result_expr->type = result_type;
  result_expr->op_kind = op_kind;
  result_expr->left = left_expr.flex();
  result_expr->right = right_expr.flex();
  return result_expr;
}

Option<Flex<Expression>> ConstIntegerType::perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
) const {
  Flex<Type> result_type = emplace_flex<ConstIntegerType>();
  switch (op_kind) {
  case UnaryOperatorKind::Negate:
    result_type = emplace_flex<ConstIntegerType>(-value);
    break;
  case UnaryOperatorKind::Positive:
    result_type = flex();
    break;
  case UnaryOperatorKind::BitwiseNot:
    result_type = emplace_flex<ConstIntegerType>(~value);
    break;
  case UnaryOperatorKind::Not:
  case UnaryOperatorKind::Increment:
  case UnaryOperatorKind::Decrement:
    return None();
  }
  auto result_expr = emplace_flex<NativeUnaryOperationExpression>();
  result_expr->node_id = expr_node_id;
  result_expr->type = result_type;
  result_expr->op_kind = op_kind;
  result_expr->operand = expr.flex();
  return result_expr;
}

bool ConstIntegerType::is_primitive() const {
  return repr_bit_size() <= 64;
}

Serialize ConstIntegerType::serialize() const {
  String repr("Const[");
  value.to_string(repr);
  repr.append(']');
  return Serialize::literal(repr);
}

} // namespace amelia
