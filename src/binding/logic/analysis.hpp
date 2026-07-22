#pragma once

#include <cstdint>

namespace amelia {

using NodeId = int32_t;
template <typename T> class Flex;
template <typename T> class Option;
struct IModuleAnalysisState;
struct FunctionParameter;
struct FunctionDefinition;
struct FunctionSignature;
struct Expression;
struct Binding;
struct ValueBinding;
struct TypeBinding;
class Text;

bool is_binding_analyzed(IModuleAnalysisState &module_state, const Binding &binding);

void get_binding_details(
    IModuleAnalysisState &module_state, Binding &current_binding_details, NodeId decl_node_id
);
void collect_top_level_bindings(IModuleAnalysisState &module_state, NodeId module_node_id);
void analyze_top_level_binding(IModuleAnalysisState &module_state, Binding &binding);

void analyze_binding(IModuleAnalysisState &module_state, Binding &binding);
void analyze_function_binding(IModuleAnalysisState &module_state, ValueBinding &binding);
void analyze_let_binding(IModuleAnalysisState &module_state, ValueBinding &binding);
void analyze_const_binding(IModuleAnalysisState &module_state, ValueBinding &binding);
void analyze_type_binding(IModuleAnalysisState &module_state, TypeBinding &binding);

FunctionParameter analyze_function_parameter(
    IModuleAnalysisState &module_state, NodeId parameter_node_id
);

Flex<FunctionSignature> analyze_function_signature(
    IModuleAnalysisState &module_state, NodeId signature_node_id
);

Flex<Expression> analyze_function_body(
    IModuleAnalysisState &module_state, FunctionSignature &signature, NodeId function_body_node_id
);

} // namespace amelia
