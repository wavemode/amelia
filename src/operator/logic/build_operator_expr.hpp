#pragma once

#include <cstdint>

#include "operator/data/binary_operator_kind.hpp"
#include "operator/data/unary_operator_kind.hpp"
#include "parser/data/node_type.hpp"

namespace amelia {

template <typename T> class Flex;
struct Expression;
struct IModuleAnalysisState;
struct Type;

using NodeId = int32_t;

Flex<Expression> build_unary_operator_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
);

Flex<Expression> build_binary_operator_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
);

Option<Flex<Expression>> perform_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Expression &left_expr,
    const Expression &right_expr
);

Option<Flex<Expression>> perform_unary_op(
    NodeId expr_node_id, UnaryOperatorKind op_kind, const Expression &operand_expr
);

Option<Flex<Expression>> perform_native_shift(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &result_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
);

Option<Flex<Expression>> perform_native_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &left_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr,
    const Type &result_type
);

Option<Flex<Expression>> perform_native_unary_op(
    NodeId expr_node_id,
    UnaryOperatorKind op_kind,
    const Type &result_type,
    const Expression &operand_expr
);

} // namespace amelia
