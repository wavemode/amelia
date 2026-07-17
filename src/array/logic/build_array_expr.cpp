#include "build_array_expr.hpp"

#include "array/data/array_expression.hpp"
#include "array/data/array_type.hpp"
#include "expr/logic/build.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "util/data/flex.hpp"

namespace amelia {

Flex<Expression> build_expr_bracket(IModuleAnalysisState &, NodeId) {
  // const auto &bracket_node = module_state.get_node(expr_node_id).as_BracketExprNode();
  // auto result = emplace_flex<ArrayLiteralExpression>();
  // result->node_id = expr_node_id;
  // auto array_type = emplace_flex<ArrayType>();
  // array_type->element_type = read_expr_list(
  //     module_state, result->elements, bracket_node.exprs.data()
  // );
  // array_type->size = result->elements.size();
  // result->type = array_type;
  // return result;
  return {};
}

} // namespace amelia
