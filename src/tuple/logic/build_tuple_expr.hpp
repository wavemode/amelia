#pragma once

#include <cstdint>

namespace amelia {

using NodeId = int32_t;
template <typename T> class Flex;
template <typename T> class ConstSlice;
struct Expression;
struct IModuleAnalysisState;

Flex<Expression> build_expr_tuple(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> expr_node_ids
);

Flex<Expression> build_expr_paren(IModuleAnalysisState &module_state, NodeId expr_node_id);

} // namespace amelia
