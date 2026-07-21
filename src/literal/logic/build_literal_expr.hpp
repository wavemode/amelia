#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Flex;
template <typename T> class ConstSlice;
struct Expression;
struct IModuleAnalysisState;

using NodeId = int32_t;

Flex<Expression> build_expr_number_literal(IModuleAnalysisState &module_state, NodeId expr_node_id);
Flex<Expression> build_expr_char_literal(IModuleAnalysisState &module_state, NodeId expr_node_id);
Flex<Expression> build_expr_string_literal(IModuleAnalysisState &module_state, NodeId expr_node_id);
Flex<Expression> build_expr_boolean_literal(
    IModuleAnalysisState &module_state, NodeId expr_node_id
);
Flex<Expression> build_expr_builtin_type(IModuleAnalysisState &module_state, NodeId expr_node_id);
Flex<Expression> build_expr_tuple(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> expr_node_ids
);
Flex<Expression> build_expr_identifier(IModuleAnalysisState &module_state, NodeId node_id);
Flex<Expression> build_expr_implicit_identifier(IModuleAnalysisState &module_state, NodeId node_id);

} // namespace amelia
