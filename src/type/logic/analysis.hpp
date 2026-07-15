#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Flex;
struct IModuleAnalysisState;
struct Type;

using NodeId = int32_t;

Flex<Type> evaluate_type_expr(IModuleAnalysisState &module_state, NodeId type_expr_node_id);

} // namespace amelia
