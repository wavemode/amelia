#pragma once

#include <cstddef>
#include <cstdint>

namespace amelia {

struct TypeBinding;
struct ValueBinding;
template <typename T> class Flex;
template <typename T> class Option;
class Text;
class Node;
struct Binding;
class String;
struct FunctionSignature;
struct ModuleAnalysisContext;
struct Module;

using NodeId = int32_t;
using BindingId = int32_t;

struct IModuleAnalysisState {
  virtual Option<BindingId> get_binding_id_by_name(Text name) = 0;
  virtual Flex<Binding> get_binding_by_id(BindingId binding_id) = 0;
  virtual const Node &get_node(NodeId node_id) const = 0;
  virtual void push_binding(Flex<Binding> binding) = 0;
  virtual Binding &pop_binding() = 0;
  virtual size_t get_binding_stack_size() const = 0;
  virtual ModuleAnalysisContext &analysis_context() = 0;
  virtual Module &current_module() = 0;
  [[noreturn]] virtual void raise_type_error_at_node(NodeId node_id, String &&message) = 0;
};

} // namespace amelia
