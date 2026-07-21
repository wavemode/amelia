#pragma once

#include <cstdint>

#include "expr/data/expression.hpp"
#include "statement/data/goto_request.hpp"
#include "type/data/type.hpp"
#include "util/data/map.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

template <typename T> class List;
struct FunctionSignature;
struct Binding;

using NodeId = int32_t;

struct ModuleAnalysisContext {
  Option<FunctionSignature *> current_function_signature;
  Option<Binding *> binding_currently_analyzing;
  Option<NodeId> intro_decls_currently_analyzing;
  Option<NodeId> loop_currently_analyzing;

  bool require_value_of_stmt = false;

  uint32_t current_scope_level = 0;
  uint32_t max_scope_level = 0;
  Map<Text, uint32_t> labels_in_scope;
  Map<Text, GotoRequest> gotos_in_scope;

  Option<NodeId> switch_case_body_stmt_node_id;
  Option<Flex<Type>> switch_case_expected_type;
  Option<List<Flex<Expression>> *> switch_case_value_exprs;
  Option<Flex<Expression>> switch_subject_expr;
};

}; // namespace amelia
