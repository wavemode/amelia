#include "build.hpp"

#include "array/logic/build_array_expr.hpp"
#include "builtin/data/builtin_type.hpp"
#include "expr/data/expression.hpp"
#include "function/logic/build_function_expr.hpp"
#include "literal/logic/build_literal_expr.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "parser/data/node.hpp"
#include "reference/logic/build_ref_expr.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "statement/logic/sequence_exprs.hpp"
#include "tuple/logic/build_tuple_expr.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/flex.hpp"
#include "util/data/serialize.hpp"
#include "util/data/string.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

Flex<Expression> build_expression(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &expr_node = module_state.get_node(expr_node_id);
  Flex<Expression> result;
  switch (expr_node.type()) {
  case NodeType::NumberLiteralNode:
    return build_expr_number_literal(module_state, expr_node_id);
  case NodeType::CharLiteralNode:
    return build_expr_char_literal(module_state, expr_node_id);
  case NodeType::StringLiteralNode:
    return build_expr_string_literal(module_state, expr_node_id);
  case NodeType::BooleanLiteralNode:
    return build_expr_boolean_literal(module_state, expr_node_id);
  case NodeType::BuiltinTypeNode:
    return build_expr_builtin_type(module_state, expr_node_id);
  case NodeType::ArrayExprNode:
    return build_expr_array(module_state, expr_node_id);
  case NodeType::ParenthesizedExprNode:
    return build_expr_paren(module_state, expr_node_id);
  case NodeType::IdentifierNode:
    result = build_expr_identifier(module_state, expr_node_id);
    break;
  case NodeType::ImplicitExprNode:
    result = build_expr_implicit_identifier(module_state, expr_node_id);
    break;
  case NodeType::NegateExprNode:
  case NodeType::PositiveExprNode:
  case NodeType::NotExprNode:
  case NodeType::BitwiseNotExprNode:
  case NodeType::PreDecrementStmtNode:
  case NodeType::PostDecrementStmtNode:
  case NodeType::PreIncrementStmtNode:
  case NodeType::PostIncrementStmtNode:
    result = build_unary_operator_expression(module_state, expr_node_id);
    break;
  case NodeType::AddExprNode:
  case NodeType::SubtractExprNode:
  case NodeType::MultiplyExprNode:
  case NodeType::DivideExprNode:
  case NodeType::AndExprNode:
  case NodeType::BitwiseAndExprNode:
  case NodeType::BitwiseOrExprNode:
  case NodeType::BitwiseXorExprNode:
  case NodeType::EqualsExprNode:
  case NodeType::GreaterExprNode:
  case NodeType::GreaterEqualsExprNode:
  case NodeType::LessExprNode:
  case NodeType::LessEqualsExprNode:
  case NodeType::LeftShiftExprNode:
  case NodeType::ModuloExprNode:
  case NodeType::NotEqualsExprNode:
  case NodeType::OrExprNode:
  case NodeType::RightShiftExprNode:
  case NodeType::AddAssignStmtNode:
  case NodeType::AssignmentStmtNode:
  case NodeType::BitwiseAndAssignStmtNode:
  case NodeType::BitwiseOrAssignStmtNode:
  case NodeType::BitwiseXorAssignStmtNode:
  case NodeType::DivAssignStmtNode:
  case NodeType::LeftShiftAssignStmtNode:
  case NodeType::ModAssignStmtNode:
  case NodeType::MulAssignStmtNode:
  case NodeType::RightShiftAssignStmtNode:
  case NodeType::SubAssignStmtNode:
    result = build_binary_operator_expression(module_state, expr_node_id);
    break;
  case NodeType::BlockExprNode:
    result = build_block_expression(module_state, expr_node_id);
    break;
  case NodeType::FunctionCallExprNode:
    result = build_funcall_expression(module_state, expr_node_id);
    break;
  case NodeType::AsExprNode:
    result = build_type_cast_expression(module_state, expr_node_id);
    break;
  case NodeType::RefExprNode:
    result = build_reference_expression(module_state, expr_node_id);
    break;
  default:
    module_state.raise_type_error_at_node(
        expr_node_id, "not implemented (unknown node type in build_expression)"
    );
  }
  return result;
}

Flex<Expression> expect_expression_of_type(
    IModuleAnalysisState &module_state, const Type &expected_type, NodeId expr_node_id
) {
  auto expr = build_expression(module_state, expr_node_id);
  return require_coerce(module_state, expected_type, *expr);
}

Flex<Expression> require_coerce(
    IModuleAnalysisState &module_state, const Type &target_type, const Expression &expr
) {
  return require_coerce(
      module_state,
      target_type,
      expr,
      "Cannot coerce expression of type '{1}' to expected type '{2}'"
  );
}

Flex<Expression> require_coerce(
    IModuleAnalysisState &module_state,
    const Type &target_type,
    const Expression &expr,
    String &&error_message_template
) {
  auto unified_expr = target_type.coerce(expr.type, expr);
  if (!unified_expr.has_value()) {
    String target_type_str;
    target_type.serialize().to_string(target_type_str);
    String expr_type_str;
    expr.type->serialize().to_string(expr_type_str);
    TextUtils::replace(error_message_template, "{1}", expr_type_str);
    TextUtils::replace(error_message_template, "{2}", target_type_str);
    module_state.raise_type_error_at_node(expr.node_id, move(error_message_template));
  }
  return unified_expr.value();
}

bool require_branches_to_have_same_type(
    Flex<Type> &result_type, Flex<Expression> &left, Flex<Expression> &right
) {
  if (is_never_type(left->type)) {
    result_type = right->type;
    return true;
  }

  if (is_never_type(right->type)) {
    result_type = left->type;
    return true;
  }

  if (left->type->unify(right->type)) {
    result_type = left->type;
    return true;
  }

  auto left_type = left->type->remove_comptime_const();
  auto right_type = right->type->remove_comptime_const();

  if (left_type->unify(right_type)) {
    result_type = left_type;
    return true;
  }

  auto coerce_right = left_type->coerce(right_type, right);
  if (coerce_right.has_value()) {
    right = coerce_right.value();
    result_type = left->type;
    return true;
  }

  return false;
}

} // namespace amelia
