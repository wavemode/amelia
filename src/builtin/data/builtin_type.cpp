#include "builtin_type.hpp"

#include "expr/data/expression.hpp"

#include "const/data/const_boolean_type.hpp"
#include "const/data/const_character_type.hpp"
#include "const/data/const_integer_type.hpp"
#include "const/data/const_rational_type.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "reference/data/reference_type.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/char_iterator.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

bool BuiltinType::unify(const Type &assignment_type) const {
  if (!assignment_type.is<BuiltinType>()) {
    return false;
  }
  return builtin_kind == assignment_type.as<BuiltinType>().builtin_kind;
}

Option<Flex<Expression>> BuiltinType::coerce(const Type &assignment_type, const Expression &expr)
    const {
  switch (builtin_kind) {
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
    return assignment_type.is_integral() &&
                   can_represent_range(assignment_type.min_value(), assignment_type.max_value())
               ? native_type_cast(*this, expr)
               : Option<Flex<Expression>>();
  case BuiltinKind::USize:
    return (is_usize_type(assignment_type) || (assignment_type.is<ConstIntegerType>() &&
                                               assignment_type.as<ConstIntegerType>().value >= 0))
               ? native_type_cast(*this, expr)
               : Option<Flex<Expression>>();
  case BuiltinKind::Float:
    return (assignment_type.is_integral() || assignment_type.is<ConstRationalType>() ||
            (assignment_type.is<BuiltinType>() &&
             (assignment_type.as<BuiltinType>().builtin_kind == BuiltinKind::Float)))
               ? native_type_cast(*this, expr)
               : Option<Flex<Expression>>();
  case BuiltinKind::Double:
    return (assignment_type.is_integral() || assignment_type.is_floating_point())
               ? native_type_cast(*this, expr)
               : Option<Flex<Expression>>();
  case BuiltinKind::Bool:
    return (assignment_type.is<ConstBooleanType>()) ||
                   (assignment_type.is<BuiltinType>() &&
                    assignment_type.as<BuiltinType>().builtin_kind == BuiltinKind::Bool)
               ? native_type_cast(*this, expr)
               : Option<Flex<Expression>>();
  case BuiltinKind::Char:
    return (assignment_type.is<ConstIntegerType>() &&
            (assignment_type.as<ConstIntegerType>().value <= UINT32_MAX &&
             assignment_type.as<ConstIntegerType>().value >= 0 &&
             CharIterator::is_valid_code_point(
                 assignment_type.as<ConstIntegerType>().value.to_uint32()
             ))) || (assignment_type.is_integral() && assignment_type.min_value() >= 0 &&
                     assignment_type.max_value() < 0xD800)
               ? native_type_cast(*this, expr)
               : Option<Flex<Expression>>();
  case BuiltinKind::Str:
  case BuiltinKind::Null:
  case BuiltinKind::Never:
  case BuiltinKind::Unknown:
    return None();
  }
}

Option<Flex<Expression>> BuiltinType::cast(const Type &source_type, const Expression &expr) const {
  if (builtin_kind != BuiltinKind::Char && has_native_numeric_repr() &&
      source_type.has_native_numeric_repr()) {
    return native_type_cast(*this, expr);
  }
  return None();
}

uint64_t BuiltinType::repr_bit_size() const {
  switch (builtin_kind) {
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
    return 8;
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
    return 16;
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Char:
  case BuiltinKind::USize:
    return 32;
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
    return 64;
  default: {
    String error_message = "Cannot determine representation bit size for builtin type '";
    serialize_builtin_kind(builtin_kind).to_string(error_message);
    error_message.append("'");
    throw RuntimeError(error_message.c_str());
  }
  }
}

Integer BuiltinType::min_value() const {
  switch (builtin_kind) {
  case BuiltinKind::Byte:
    return Integer(INT8_MIN);
  case BuiltinKind::UByte:
    return Integer(0);
  case BuiltinKind::Short:
    return Integer(INT16_MIN);
  case BuiltinKind::UShort:
    return Integer(0);
  case BuiltinKind::Int:
    return Integer(INT32_MIN);
  case BuiltinKind::UInt:
    return Integer(0);
  case BuiltinKind::Long:
    return Integer(INT64_MIN);
  case BuiltinKind::ULong:
    return Integer(0);
  case BuiltinKind::USize:
    return Integer(0);
  case BuiltinKind::Char:
    return Integer(0);
  default: {
    String error_message = "Cannot determine minimum value for builtin type '";
    serialize_builtin_kind(builtin_kind).to_string(error_message);
    error_message.append("'");
    throw RuntimeError(error_message.c_str());
  }
  }
}

Integer BuiltinType::max_value() const {
  switch (builtin_kind) {
  case BuiltinKind::Byte:
    return Integer(INT8_MAX);
  case BuiltinKind::UByte:
    return Integer(UINT8_MAX);
  case BuiltinKind::Short:
    return Integer(INT16_MAX);
  case BuiltinKind::UShort:
    return Integer(UINT16_MAX);
  case BuiltinKind::Int:
    return Integer(INT32_MAX);
  case BuiltinKind::UInt:
    return Integer(UINT32_MAX);
  case BuiltinKind::Long:
    return Integer(INT64_MAX);
  case BuiltinKind::ULong:
  case BuiltinKind::USize:
    return Integer(UINT64_MAX);
  case BuiltinKind::Char:
    return Integer(0x10FFFF);
  default: {
    String error_message = "Cannot determine maximum value for builtin type '";
    serialize_builtin_kind(builtin_kind).to_string(error_message);
    error_message.append("'");
    throw RuntimeError(error_message.c_str());
  }
  }
}

static Option<Flex<Expression>> perform_binary_op_on_integral(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &left_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) {
  switch (op_kind) {
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::RightShift:
    return perform_native_shift(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, left_type
    );
  case BinaryOperatorKind::Add:
  case BinaryOperatorKind::Subtract:
  case BinaryOperatorKind::Multiply:
  case BinaryOperatorKind::Divide:
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
  case BinaryOperatorKind::Modulo:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, left_type
    );
  case BinaryOperatorKind::Equals:
  case BinaryOperatorKind::Greater:
  case BinaryOperatorKind::GreaterEquals:
  case BinaryOperatorKind::Less:
  case BinaryOperatorKind::LessEquals:
  case BinaryOperatorKind::NotEquals:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, BOOL_TYPE
    );
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
    return perform_native_shift(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, NULL_TYPE
    );
  case BinaryOperatorKind::Assignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::AddAssignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::MulAssignment:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, NULL_TYPE
    );
  case BinaryOperatorKind::Or:
  case BinaryOperatorKind::And:
    return None();
  }
}

static Option<Flex<Expression>> perform_binary_op_on_char(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &left_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) {
  switch (op_kind) {
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::RightShift:
  case BinaryOperatorKind::Add:
  case BinaryOperatorKind::Subtract:
  case BinaryOperatorKind::Multiply:
  case BinaryOperatorKind::Divide:
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
  case BinaryOperatorKind::Modulo:
    return UINT_TYPE->perform_binary_op(expr_node_id, op_kind, left_expr, right_type, right_expr);
  case BinaryOperatorKind::Equals:
  case BinaryOperatorKind::Greater:
  case BinaryOperatorKind::GreaterEquals:
  case BinaryOperatorKind::Less:
  case BinaryOperatorKind::LessEquals:
  case BinaryOperatorKind::NotEquals:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, BOOL_TYPE
    );
  case BinaryOperatorKind::Assignment:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, NULL_TYPE
    );
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::AddAssignment:
  case BinaryOperatorKind::Or:
  case BinaryOperatorKind::And:
    return None();
  }
}

static Option<Flex<Expression>> perform_binary_op_on_floating_point(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &left_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) {
  switch (op_kind) {
  case BinaryOperatorKind::Add:
  case BinaryOperatorKind::Subtract:
  case BinaryOperatorKind::Multiply:
  case BinaryOperatorKind::Divide:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, left_type
    );
  case BinaryOperatorKind::Equals:
  case BinaryOperatorKind::Greater:
  case BinaryOperatorKind::GreaterEquals:
  case BinaryOperatorKind::Less:
  case BinaryOperatorKind::LessEquals:
  case BinaryOperatorKind::NotEquals:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, BOOL_TYPE
    );
  case BinaryOperatorKind::Assignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::AddAssignment:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, NULL_TYPE
    );
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
  case BinaryOperatorKind::Modulo:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::RightShift:
  case BinaryOperatorKind::Or:
  case BinaryOperatorKind::And:
    return None();
  }
}

static Option<Flex<Expression>> perform_binary_op_on_boolean(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &left_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) {
  switch (op_kind) {
  case BinaryOperatorKind::Equals:
  case BinaryOperatorKind::NotEquals:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, BOOL_TYPE
    );
  case BinaryOperatorKind::Assignment:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, NULL_TYPE
    );
  case BinaryOperatorKind::Or:
  case BinaryOperatorKind::And:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, left_type
    );
  case BinaryOperatorKind::Add:
  case BinaryOperatorKind::Subtract:
  case BinaryOperatorKind::Multiply:
  case BinaryOperatorKind::Divide:
  case BinaryOperatorKind::Greater:
  case BinaryOperatorKind::GreaterEquals:
  case BinaryOperatorKind::Less:
  case BinaryOperatorKind::LessEquals:
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
  case BinaryOperatorKind::Modulo:
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::RightShift:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::AddAssignment:
    return None();
  }
}

static Option<Flex<Expression>> perform_binary_op_on_null(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &left_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) {
  switch (op_kind) {
  case BinaryOperatorKind::Equals:
  case BinaryOperatorKind::NotEquals:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, BOOL_TYPE
    );
  case BinaryOperatorKind::Assignment:
    return perform_native_binary_op(
        expr_node_id, op_kind, left_type, left_expr, right_type, right_expr, NULL_TYPE
    );
  case BinaryOperatorKind::Or:
  case BinaryOperatorKind::And:
  case BinaryOperatorKind::Add:
  case BinaryOperatorKind::Subtract:
  case BinaryOperatorKind::Multiply:
  case BinaryOperatorKind::Divide:
  case BinaryOperatorKind::Greater:
  case BinaryOperatorKind::GreaterEquals:
  case BinaryOperatorKind::Less:
  case BinaryOperatorKind::LessEquals:
  case BinaryOperatorKind::BitwiseAnd:
  case BinaryOperatorKind::BitwiseOr:
  case BinaryOperatorKind::BitwiseXor:
  case BinaryOperatorKind::Modulo:
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::RightShift:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::AddAssignment:
    return None();
  }
}

Option<Flex<Expression>> BuiltinType::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  switch (builtin_kind) {
  case BuiltinKind::Char:
    return UINT_TYPE->perform_binary_op(expr_node_id, op_kind, left_expr, right_type, right_expr);
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
  case BuiltinKind::USize:
    return perform_binary_op_on_integral(
        expr_node_id, op_kind, *this, left_expr, right_type, right_expr
    );
  case BuiltinKind::Bool:
    return perform_binary_op_on_boolean(
        expr_node_id, op_kind, *this, left_expr, right_type, right_expr
    );
  case BuiltinKind::Float:
  case BuiltinKind::Double:
    return perform_binary_op_on_floating_point(
        expr_node_id, op_kind, *this, left_expr, right_type, right_expr
    );
  case BuiltinKind::Null:
    return perform_binary_op_on_null(
        expr_node_id, op_kind, *this, left_expr, right_type, right_expr
    );
  case BuiltinKind::Str:
  case BuiltinKind::Never:
  case BuiltinKind::Unknown:
    return None();
  }
}

static Option<Flex<Expression>> perform_unary_op_on_integral(
    NodeId expr_node_id,
    UnaryOperatorKind op_kind,
    const Type &operand_type,
    const Expression &operand_expr
) {
  switch (op_kind) {
  case UnaryOperatorKind::Negate:
    if (operand_type.min_value() == 0) {
      return None();
    }
    [[fallthrough]];
  case UnaryOperatorKind::Positive:
  case UnaryOperatorKind::BitwiseNot:
    return perform_native_unary_op(expr_node_id, op_kind, operand_type, operand_expr, operand_type);
  case UnaryOperatorKind::Increment:
  case UnaryOperatorKind::Decrement:
    return perform_native_unary_op(expr_node_id, op_kind, operand_type, operand_expr, NULL_TYPE);
  case UnaryOperatorKind::Not:
    return None();
  }
}

static Option<Flex<Expression>> perform_unary_op_on_floating_point(
    NodeId expr_node_id,
    UnaryOperatorKind op_kind,
    const Type &operand_type,
    const Expression &operand_expr
) {
  switch (op_kind) {
  case UnaryOperatorKind::Negate:
  case UnaryOperatorKind::Positive:
    return perform_native_unary_op(expr_node_id, op_kind, operand_type, operand_expr, operand_type);
  case UnaryOperatorKind::Increment:
  case UnaryOperatorKind::Decrement:
    return perform_native_unary_op(expr_node_id, op_kind, operand_type, operand_expr, NULL_TYPE);
  case UnaryOperatorKind::Not:
  case UnaryOperatorKind::BitwiseNot:
    return None();
  }
}

static Option<Flex<Expression>> perform_unary_op_on_boolean(
    NodeId expr_node_id,
    UnaryOperatorKind op_kind,
    const Type &operand_type,
    const Expression &operand_expr
) {
  switch (op_kind) {
  case UnaryOperatorKind::Not:
    return perform_native_unary_op(expr_node_id, op_kind, operand_type, operand_expr, operand_type);
  case UnaryOperatorKind::Increment:
  case UnaryOperatorKind::Decrement:
  case UnaryOperatorKind::Negate:
  case UnaryOperatorKind::Positive:
  case UnaryOperatorKind::BitwiseNot:
    return None();
  }
}

static Option<Flex<Expression>> perform_unary_op_on_char(
    NodeId expr_node_id,
    UnaryOperatorKind op_kind,
    const Type &operand_type,
    const Expression &operand_expr
) {
  switch (op_kind) {
  case UnaryOperatorKind::Positive:
    return perform_native_unary_op(expr_node_id, op_kind, operand_type, operand_expr, operand_type);
  case UnaryOperatorKind::BitwiseNot:
    return UINT_TYPE->perform_unary_op(expr_node_id, op_kind, operand_expr);
  case UnaryOperatorKind::Not:
  case UnaryOperatorKind::Increment:
  case UnaryOperatorKind::Decrement:
  case UnaryOperatorKind::Negate:
    return None();
  }
}

Option<Flex<Expression>> BuiltinType::perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &expr
) const {
  switch (builtin_kind) {
  case BuiltinKind::Char:
    return perform_unary_op_on_char(expr_node_id, op_kind, *this, expr);
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
  case BuiltinKind::USize:
    return perform_unary_op_on_integral(expr_node_id, op_kind, *this, expr);
  case BuiltinKind::Bool:
    return perform_unary_op_on_boolean(expr_node_id, op_kind, *this, expr);
  case BuiltinKind::Float:
  case BuiltinKind::Double:
    return perform_unary_op_on_floating_point(expr_node_id, op_kind, *this, expr);
  case BuiltinKind::Str:
  case BuiltinKind::Null:
  case BuiltinKind::Never:
  case BuiltinKind::Unknown:
    return None();
  }
}

bool BuiltinType::is_primitive() const {
  switch (builtin_kind) {
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
  case BuiltinKind::Char:
  case BuiltinKind::USize:
  case BuiltinKind::Bool:
  case BuiltinKind::Float:
  case BuiltinKind::Double:
    return true;
  case BuiltinKind::Str:
  case BuiltinKind::Null:
  case BuiltinKind::Never:
  case BuiltinKind::Unknown:
    return false;
  }
}

bool BuiltinType::is_integral() const {
  switch (builtin_kind) {
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
  case BuiltinKind::Char:
  case BuiltinKind::USize:
    return true;
  case BuiltinKind::Bool:
  case BuiltinKind::Float:
  case BuiltinKind::Double:
  case BuiltinKind::Str:
  case BuiltinKind::Null:
  case BuiltinKind::Never:
  case BuiltinKind::Unknown:
    return false;
  }
}

bool BuiltinType::is_floating_point() const {
  switch (builtin_kind) {
  case BuiltinKind::Float:
  case BuiltinKind::Double:
    return true;
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
  case BuiltinKind::USize:
  case BuiltinKind::Bool:
  case BuiltinKind::Char:
  case BuiltinKind::Str:
  case BuiltinKind::Null:
  case BuiltinKind::Never:
  case BuiltinKind::Unknown:
    return false;
  }
}

bool BuiltinType::has_native_numeric_repr() const {
  switch (builtin_kind) {
  case BuiltinKind::Byte:
  case BuiltinKind::UByte:
  case BuiltinKind::Short:
  case BuiltinKind::UShort:
  case BuiltinKind::Int:
  case BuiltinKind::UInt:
  case BuiltinKind::Long:
  case BuiltinKind::ULong:
  case BuiltinKind::USize:
  case BuiltinKind::Float:
  case BuiltinKind::Double:
  case BuiltinKind::Bool:
  case BuiltinKind::Char:
  case BuiltinKind::Null:
    return true;
  case BuiltinKind::Str:
  case BuiltinKind::Never:
  case BuiltinKind::Unknown:
    return false;
  }
}

Serialize BuiltinType::serialize() const {
  return serialize_builtin_kind(builtin_kind);
}

BuiltinType::BuiltinType(BuiltinKind kind) : builtin_kind(kind) {}

Flex<Type> BYTE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Byte);

Flex<Type> UBYTE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::UByte);

Flex<Type> SHORT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Short);

Flex<Type> USHORT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::UShort);

Flex<Type> INT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Int);

Flex<Type> UINT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::UInt);

Flex<Type> LONG_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Long);

Flex<Type> ULONG_TYPE = emplace_flex<BuiltinType>(BuiltinKind::ULong);

Flex<Type> USIZE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::USize);

Flex<Type> FLOAT_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Float);

Flex<Type> DOUBLE_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Double);

Flex<Type> BOOL_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Bool);

Flex<Type> CHAR_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Char);

Flex<Type> STR_TYPE;

Flex<Type> STR_REF_TYPE = [] {
  new (&STR_TYPE) Flex<Type>(emplace_flex<BuiltinType>(BuiltinKind::Str));
  auto result = emplace_flex<ReferenceType>();
  result->referent = STR_TYPE;
  result->is_const = false;
  return result;
}();

Flex<Type> NULL_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Null);

Flex<Type> NEVER_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Never);

Flex<Type> UNKNOWN_TYPE = emplace_flex<BuiltinType>(BuiltinKind::Unknown);

bool is_unknown_type(const Type &type) {
  return type.is<BuiltinType>() && type.as<BuiltinType>().builtin_kind == BuiltinKind::Unknown;
}

bool is_never_type(const Type &type) {
  return type.is<BuiltinType>() && type.as<BuiltinType>().builtin_kind == BuiltinKind::Never;
}

bool is_null_type(const Type &type) {
  return type.is<BuiltinType>() && type.as<BuiltinType>().builtin_kind == BuiltinKind::Null;
}

bool is_float_type(const Type &type) {
  return type.is<BuiltinType>() && type.as<BuiltinType>().builtin_kind == BuiltinKind::Float;
}

bool is_double_type(const Type &type) {
  return type.is<BuiltinType>() && type.as<BuiltinType>().builtin_kind == BuiltinKind::Double;
}

bool is_usize_type(const Type &type) {
  return type.is<BuiltinType>() && type.as<BuiltinType>().builtin_kind == BuiltinKind::USize;
}

} // namespace amelia
