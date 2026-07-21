#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Flex;
template <typename T> class Option;
struct Expression;
struct IModuleAnalysisState;
struct Type;

using NodeId = int32_t;

Flex<Expression> build_type_cast_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
);

Flex<Expression> native_type_cast(const Type &target_type, const Expression &expr);

Option<Flex<Expression>> require_boolean_expr(const Expression &expr);

} // namespace amelia
