#include "build_tuple_expr.hpp"

#include "builtin/data/builtin_type.hpp"
#include "expr/logic/build.hpp"
#include "literal/data/null_literal_expression.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "tuple/data/tuple_expression.hpp"
#include "tuple/data/tuple_type.hpp"
#include "util/data/list.hpp"
#include "util/data/slice.hpp"

namespace amelia {

namespace {

Flex<Expression> build_expr_tuple(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> expr_node_ids
) {
  auto result = emplace_flex<TupleExpression>();
  result->node_id = expr_node_id;
  for (NodeId sub_expr_node_id : expr_node_ids) {
    result->elements.push_back(build_expression(module_state, sub_expr_node_id));
  }
  auto tuple_type = emplace_flex<TupleType>();
  for (const auto &element : result->elements) {
    tuple_type->element_types.push_back(element->type);
  }
  result->type = tuple_type;
  return result;
}

} // namespace

Flex<Expression> build_expr_paren(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &paren_node = module_state.get_node(expr_node_id).as_ParenthesizedExprNode();
  if (paren_node.exprs.size() == 0) {
    auto result = emplace_flex<NullLiteralExpression>();
    result->node_id = expr_node_id;
    result->type = NULL_TYPE;
    return result;
  } else if (paren_node.exprs.size() == 1) {
    return build_expression(module_state, paren_node.exprs[0]);
  } else {
    return build_expr_tuple(module_state, expr_node_id, paren_node.exprs.data());
  }
}

} // namespace amelia
