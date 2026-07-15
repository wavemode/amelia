#include "sequence_exprs.hpp"

#include "expr/data/expression.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "util/data/flex.hpp"

namespace amelia {

Flex<Expression> build_block_expression(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  module_state.raise_error_at_node(
      expr_node_id, "not implemented (unknown node type in build_block_expression)"
  );
}

} // namespace amelia
