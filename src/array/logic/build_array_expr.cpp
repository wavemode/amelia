#include "build_array_expr.hpp"

#include "array/data/array_expression.hpp"
#include "array/data/array_type.hpp"
#include "const/data/const_integer_type.hpp"
#include "expr/logic/build.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "type/logic/analysis.hpp"
#include "util/data/flex.hpp"
#include "util/data/text_utils.hpp"

namespace amelia {

Flex<Expression> build_expr_array(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &array_node = module_state.get_node(expr_node_id).as_ArrayExprNode();

  if (!array_node.elements.has_value()) {
    module_state.raise_type_error_at_node(
        expr_node_id, "Invalid expression (array literal with no braces)"
    );
  }

  auto element_type = evaluate_type_expr(module_state, array_node.type_expr);

  auto result = emplace_flex<ArrayLiteralExpression>();
  result->node_id = expr_node_id;
  for (NodeId element_node_id : array_node.elements.value()) {
    result->elements.push_back(
        require_coerce(module_state, element_type, build_expression(module_state, element_node_id))
    );
  }

  auto array_type = emplace_flex<ArrayType>();
  array_type->element_type = element_type;

  if (array_node.infer_size) {
    array_type->size = static_cast<uint64_t>(result->elements.size());
  } else {
    if (!array_node.size.has_value()) {
      module_state.raise_type_error_at_node(
          expr_node_id, "Invalid expression (array literal with no size)"
      );
    }

    uint64_t size_value = evaluate_array_size_expr(module_state, array_node.size.value());
    if (size_value != result->elements.size()) {
      String error_message = "Array literal has ";
      ;
      TextUtils::to_string(error_message, result->elements.size());
      error_message.append(" elements, but the specified size is ");
      TextUtils::to_string(error_message, size_value);
      module_state.raise_type_error_at_node(array_node.size.value(), move(error_message));
    }
    array_type->size = size_value;
  }
  result->type = array_type;
  return result;
}

uint64_t evaluate_array_size_expr(IModuleAnalysisState &module_state, NodeId size_expr_node_id) {
  auto size_expr = build_expression(module_state, size_expr_node_id);
  if (!size_expr->type->is<ConstIntegerType>()) {
    module_state.raise_type_error_at_node(
        size_expr_node_id, "Array size must be a constant integer"
    );
  }

  const Integer &size_value = size_expr->type->as<ConstIntegerType>().value;
  if (size_value <= 0 || size_value > UINT64_MAX) {
    module_state.raise_type_error_at_node(
        size_expr_node_id, "Array size must be a positive integer less than or equal to UINT64_MAX"
    );
  }
  return size_value.to_uint64();
}

} // namespace amelia
