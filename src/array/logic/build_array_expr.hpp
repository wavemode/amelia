#pragma once

#include <cstdint>

namespace amelia {

using NodeId = int32_t;
template <typename T> class Flex;
template <typename T> class ConstSlice;
struct Expression;
struct IModuleAnalysisState;

Flex<Expression> build_expr_bracket(IModuleAnalysisState &module_state, NodeId expr_node_id);

} // namespace amelia
