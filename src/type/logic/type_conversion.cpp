#include "type_conversion.hpp"

#include "alias/data/alias_type.hpp"
#include "builtin/data/builtin_type.hpp"
#include "const/data/const_boolean_type.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "tuple/data/tuple_type.hpp"
#include "type/data/native_type_cast_expression.hpp"
#include "type/logic/analysis.hpp"
#include "util/data/string.hpp"

namespace amelia {
Flex<Expression> build_type_cast_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const auto &as_node = module_state.get_node(expr_node_id).as_AsExprNode();
  auto expr = build_expression(module_state, as_node.expr);
  auto target_type = evaluate_type_expr(module_state, as_node.type);
  auto result = target_type->cast(expr->type, expr);

  if (!result.has_value()) {
    String error_message = "Cannot cast expression of type '";
    expr->type->serialize().to_string(error_message);
    error_message.append("' to type '");
    target_type->serialize().to_string(error_message);
    error_message.append("'");
    module_state.raise_type_error_at_node(expr_node_id, move(error_message));
  }

  // in case the type coerced without actually changing
  result.value()->type = target_type;

  return result.value();
}

Flex<Expression> native_type_cast(const Type &target_type, const Expression &expr) {
  auto coerce_expr = emplace_flex<NativeTypeCastExpression>();
  coerce_expr->type = target_type.flex();
  coerce_expr->expr = expr.flex();
  return coerce_expr;
}

Option<Flex<Expression>> require_boolean_expr(const Expression &expr) {
  auto expr_type = expr.type->resolve();
  if (expr_type->is<ConstBooleanType>() || is_bool_type(expr_type)) {
    return expr.flex();
  }

  return BOOL_TYPE->coerce(expr);
}

} // namespace amelia
