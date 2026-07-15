#include "build_literal_expr.hpp"

#include "binding/data/value_binding.hpp"
#include "binding/logic/analysis.hpp"
#include "builtin/data/builtin_type.hpp"
#include "const/data/const_boolean_type.hpp"
#include "const/data/const_character_type.hpp"
#include "const/data/const_integer_type.hpp"
#include "const/data/const_rational_type.hpp"
#include "const/data/const_string_type.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "literal/data/boolean_literal_expression.hpp"
#include "literal/data/char_literal_expression.hpp"
#include "literal/data/identifier_expression.hpp"
#include "literal/data/null_literal_expression.hpp"
#include "literal/data/number_literal_expression.hpp"
#include "literal/data/string_literal_expression.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "util/data/flex.hpp"

namespace amelia {

Flex<Expression> build_expr_number_literal(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  auto expr = emplace_flex<NumberLiteralExpression>();
  const auto &expr_node = module_state.get_node(expr_node_id).as_NumberLiteralNode();
  expr->node_id = expr_node_id;
  expr->value = expr_node.value;
  if (expr_node.value.has_decimal_point || expr_node.value.exponent_sign == "-") {
    String num_str;
    num_str.append(expr_node.value.base_prefix);
    num_str.append(expr_node.value.integer_digits);
    num_str.append(".");
    num_str.append(expr_node.value.fractional_digits);
    Rational result(num_str);
    if (expr_node.value.exponent_digits.size() > 0) {
      uint8_t exponent_base = 10;
      if (expr_node.value.exponent_prefix == "p" || expr_node.value.exponent_prefix == "P") {
        exponent_base = 2;
      }
      auto factor = Integer(exponent_base)
                        .pow(Integer(expr_node.value.exponent_digits).to_uint32());
      if (expr_node.value.exponent_sign == "-") {
        result = Rational(result.numerator(), result.denominator() * factor);
      } else {
        result = Rational(result.numerator() * factor, result.denominator());
      }
    }
    expr->type = make_flex(ConstRationalType(move(result)));
  } else {
    String num_str;
    num_str.append(expr_node.value.base_prefix);
    num_str.append(expr_node.value.integer_digits);
    Integer result(num_str);
    if (expr_node.value.exponent_digits.size() > 0) {
      uint8_t exponent_base = 10;
      if (expr_node.value.exponent_prefix == "p" || expr_node.value.exponent_prefix == "P") {
        exponent_base = 2;
      }
      auto factor = Integer(exponent_base)
                        .pow(Integer(expr_node.value.exponent_digits).to_uint32());
      result *= factor;
    }
    expr->type = make_flex(ConstIntegerType(move(result)));
  }
  return expr;
}

Flex<Expression> build_expr_char_literal(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &char_node = module_state.get_node(expr_node_id).as_CharLiteralNode();
  auto result = emplace_flex<CharLiteralExpression>();
  result->node_id = expr_node_id;
  auto type = emplace_flex<ConstCharacterType>();
  type->value = char_node.code_point;
  result->type = type;
  result->value = char_node.code_point;
  return result;
}

Flex<Expression> build_expr_string_literal(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const auto &string_node = module_state.get_node(expr_node_id).as_StringLiteralNode();
  auto result = emplace_flex<StringLiteralExpression>();
  result->node_id = expr_node_id;
  auto type = emplace_flex<ConstStringType>();
  type->value = string_node.contents;
  result->type = type;
  result->value = string_node.contents;
  return result;
}

Flex<Expression> build_expr_boolean_literal(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const Node &node = module_state.get_node(expr_node_id);
  const BooleanLiteralNode &expr_node = node.as_BooleanLiteralNode();
  auto result = emplace_flex<BooleanLiteralExpression>();
  result->node_id = expr_node_id;
  result->value = expr_node.value;
  result->type = make_flex(ConstBooleanType(expr_node.value));
  return result;
}

Flex<Expression> build_expr_builtin_type(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const Node &node = module_state.get_node(expr_node_id);
  const BuiltinTypeNode &expr_node = node.as_BuiltinTypeNode();
  switch (expr_node.kind) {
  case BuiltinKind::Null: {
    auto result = emplace_flex<NullLiteralExpression>();
    result->node_id = expr_node_id;
    result->type = NULL_TYPE;
    return result;
  }
  default:
    module_state.raise_error_at_node(expr_node_id, "not implemented");
  }
}

Flex<Expression> build_expr_identifier(IModuleAnalysisState &module_state, NodeId node_id) {
  const auto &identifier_node = module_state.get_node(node_id).as_IdentifierNode();
  auto expr = emplace_flex<IdentifierExpression>();
  auto binding = resolve_value_binding(module_state, node_id, identifier_node.name);
  expr->binding = binding.weak();
  expr->type = binding->type.value().weak();
  expr->node_id = node_id;
  return expr;
}

} // namespace amelia
