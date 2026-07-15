#include "const_string_type.hpp"

#include "builtin/data/builtin_type.hpp"
#include "const/data/const_boolean_type.hpp"
#include "const/data/const_character_type.hpp"
#include "operator/data/native_binary_operation.hpp"
#include "source/data/char_literal.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/char_iterator.hpp"
#include "util/data/flex.hpp"
#include "util/data/integer.hpp"
#include "util/data/option.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"

namespace amelia {

ConstStringType::ConstStringType() {}

ConstStringType::ConstStringType(String &&val) : value(move(val)) {}

bool ConstStringType::is_comptime_const() const {
  return true;
}

Flex<Type> ConstStringType::remove_comptime_const() const {
  return STR_REF_TYPE;
}

bool ConstStringType::unify(const Type &assignment_type) const {
  return assignment_type.is<ConstStringType>() &&
         value == assignment_type.as<ConstStringType>().value;
}

Option<Flex<Expression>> ConstStringType::perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) const {
  if (right_type.is<ConstStringType>()) {
    auto &right_const_string_type = right_type.as<ConstStringType>();
    auto result = emplace_flex<NativeBinaryOperationExpression>();
    result->node_id = expr_node_id;
    if (op_kind == BinaryOperatorKind::Add) {
      result->type = emplace_flex<ConstStringType>(value + right_const_string_type.value);
      result->op_kind = BinaryOperatorKind::Add;
    } else if (op_kind == BinaryOperatorKind::Equals) {
      result->type = emplace_flex<ConstBooleanType>(value == right_const_string_type.value);
      result->op_kind = BinaryOperatorKind::Equals;
    } else if (op_kind == BinaryOperatorKind::NotEquals) {
      result->type = emplace_flex<ConstBooleanType>(value != right_const_string_type.value);
      result->op_kind = BinaryOperatorKind::NotEquals;
    } else if (op_kind == BinaryOperatorKind::Less) {
      result->type = emplace_flex<ConstBooleanType>(value < right_const_string_type.value);
      result->op_kind = BinaryOperatorKind::Less;
    } else if (op_kind == BinaryOperatorKind::LessEquals) {
      result->type = emplace_flex<ConstBooleanType>(value <= right_const_string_type.value);
      result->op_kind = BinaryOperatorKind::LessEquals;
    } else if (op_kind == BinaryOperatorKind::Greater) {
      result->type = emplace_flex<ConstBooleanType>(value > right_const_string_type.value);
      result->op_kind = BinaryOperatorKind::Greater;
    } else if (op_kind == BinaryOperatorKind::GreaterEquals) {
      result->type = emplace_flex<ConstBooleanType>(value >= right_const_string_type.value);
      result->op_kind = BinaryOperatorKind::GreaterEquals;
    } else {
      return None();
    }
    result->left = left_expr.flex();
    result->right = right_expr.flex();
    return result;
  } else if (right_type.is<ConstCharacterType>()) {
    auto &right_const_character_type = right_type.as<ConstCharacterType>();
    auto result = emplace_flex<NativeBinaryOperationExpression>();
    result->node_id = expr_node_id;
    if (op_kind == BinaryOperatorKind::Add) {
      String repr;
      repr.append(value);
      repr.append(right_const_character_type.value);
      result->type = emplace_flex<ConstStringType>(move(repr));
      result->op_kind = BinaryOperatorKind::Add;
    } else {
      return None();
    }
    result->left = left_expr.flex();
    result->right = right_expr.flex();
    return result;
  } else {
    return None();
  }
}
Serialize ConstStringType::serialize() const {
  String repr("Const[\"");
  for (uint32_t ch : value) {
    if (ch == '"') {
      repr.append("\\\"");
    } else {
      serialize_char_literal(ch, false).to_string(repr);
    }
  }
  repr.append("\"]");
  return Serialize::literal(move(repr));
}

} // namespace amelia
