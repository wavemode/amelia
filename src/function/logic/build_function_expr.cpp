#include "build_function_expr.hpp"

#include "expr/data/expression.hpp"
#include "literal/logic/build_literal_expr.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "statement/logic/sequence_exprs.hpp"
#include "util/data/flex.hpp"

namespace amelia {

Flex<Expression> build_funcall_expression(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  module_state.raise_error_at_node(
      expr_node_id, "not implemented (unknown node type in build_funcall_expression)"
  );
}

} // namespace amelia
