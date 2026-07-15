#pragma once

#include <cstdint>

#include "parser/data/node_type.hpp"

namespace amelia {

template <typename T> class Flex;
template <typename T> class ConstSlice;
struct Expression;
struct IModuleAnalysisState;

using NodeId = int32_t;

bool is_binding_node_type(NodeType node_type);

Flex<Expression> build_statement(IModuleAnalysisState &module_state, NodeId expr_node_id);

Flex<Expression> build_expr_expression_statement(
    IModuleAnalysisState &module_state, NodeId expr_node_id
);

Flex<Expression> build_expr_return(IModuleAnalysisState &module_state, NodeId expr_node_id);

Flex<Expression> assign_current_function_return_value(
    IModuleAnalysisState &module_state, Flex<Expression> return_value
);

Flex<Expression> build_expr_type_decl(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
);

Flex<Expression> build_expr_type_binding(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
);

Flex<Expression> build_expr_fun_decl(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
);

Flex<Expression> build_expr_var_decl(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
);

Flex<Expression> build_expr_value_binding(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
);

Flex<Expression> build_expr_binding(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
);

Flex<Expression> build_expr_seq(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
);

Flex<Expression> build_block_expression(IModuleAnalysisState &module_state, NodeId expr_node_id);

} // namespace amelia
