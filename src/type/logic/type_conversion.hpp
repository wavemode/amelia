#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Flex;
struct Expression;
struct IModuleAnalysisState;
struct Type;

using NodeId = int32_t;

Flex<Expression> build_type_cast_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
);

Flex<Expression> native_type_cast(const Type &target_type, const Expression &expr);

} // namespace amelia
