#pragma once

#include <cstdint>

#include "statement/data/goto_request.hpp"
#include "util/data/map.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct FunctionSignature;
struct Binding;

using NodeId = int32_t;

struct ModuleAnalysisContext {
  Option<FunctionSignature *> current_function_signature;
  Option<Binding *> binding_currently_analyzing;
  Option<NodeId> intro_decls_currently_analyzing;
  Option<NodeId> loop_currently_analyzing;

  bool need_value_of_stmt = false;

  uint32_t current_scope_level = 0;
  uint32_t max_scope_level = 0;
  Map<Text, uint32_t> labels_in_scope;
  Map<Text, GotoRequest> gotos_in_scope;
};

}; // namespace amelia
