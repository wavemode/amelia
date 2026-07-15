#pragma once

#include <cstdint>

namespace amelia {

class String;
struct Type;
template <typename T> class Flex;
template <typename T> class List;
template <typename T> class ConstSlice;
struct Expression;
struct IModuleAnalysisState;

using NodeId = int32_t;

Flex<Expression> build_expression(IModuleAnalysisState &module_state, NodeId expr_node_id);
Flex<Expression> expect_expression_of_type(
    IModuleAnalysisState &module_state, const Type &expected_type, NodeId expr_node_id
);

Flex<Expression> require_coerce(
    IModuleAnalysisState &module_state, const Type &target_type, const Expression &expr
);

Flex<Expression> require_coerce(
    IModuleAnalysisState &module_state,
    const Type &target_type,
    const Expression &expr,
    String &&error_message_template
);

Flex<Type> read_expr_list(
    IModuleAnalysisState &module_state,
    List<Flex<Expression>> &output,
    ConstSlice<NodeId> expr_node_ids
);

} // namespace amelia
