#pragma once

#include <cstddef>
#include <cstdint>

namespace amelia {

struct TypeBinding;
struct ValueBinding;
template <typename T> class Flex;
class Text;
class Node;

using NodeId = int32_t;

struct IModuleAnalysisState {
  virtual Flex<TypeBinding> resolve_type_binding(NodeId node_id, Text name) = 0;
  virtual Flex<ValueBinding> resolve_value_binding(NodeId node_id, Text name) = 0;
  virtual const Node &get_node(NodeId node_id) const = 0;
  [[noreturn]] virtual void raise_error_at_node(NodeId node_id, String &&message) = 0;
};

} // namespace amelia
