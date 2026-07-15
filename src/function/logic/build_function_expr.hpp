#pragma once

#include <cstdint>

namespace amelia {

template <typename T> class Flex;
template <typename T> class Slice;
template <typename T> class Option;
template <typename K, typename V> class Map;
class Text;
struct Expression;
struct IModuleAnalysisState;
struct FunctionCallExpression;

using NodeId = int32_t;

Flex<Expression> build_funcall_expression(IModuleAnalysisState &module_state, NodeId expr_node_id);

Option<Flex<FunctionCallExpression>> resolve_function_call(
    IModuleAnalysisState &module_state,
    NodeId expr_node_id,
    Flex<Expression> callee,
    Slice<Flex<Expression>> pos_args,
    const Map<Text, Flex<Expression>> &named_args
);

} // namespace amelia
