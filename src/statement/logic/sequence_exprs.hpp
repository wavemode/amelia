#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Flex;
template <typename T> class ConstSlice;
struct Expression;
struct IModuleAnalysisState;

using NodeId = int32_t;

Flex<Expression> build_stmt_seq(
    IModuleAnalysisState &module_state,
    NodeId expr_node_id,
    ConstSlice<NodeId> stmts
);

Flex<Expression> build_block_expression(IModuleAnalysisState &module_state, NodeId expr_node_id);

} // namespace amelia
