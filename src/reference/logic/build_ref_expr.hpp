#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Flex;
struct Expression;
struct IModuleAnalysisState;

using NodeId = int32_t;

Flex<Expression> build_reference_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
);

} // namespace amelia
