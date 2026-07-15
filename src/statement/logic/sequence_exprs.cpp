#include "sequence_exprs.hpp"

#include "builtin/data/builtin_type.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "parser/data/node.hpp"
#include "sema/data/binding.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "statement/data/statement_sequence.hpp"
#include "statement/data/type_binding_statement.hpp"
#include "statement/data/value_binding_statement.hpp"
#include "type/logic/analysis.hpp"
#include "util/data/flex.hpp"
#include "util/data/slice_utils.hpp"

namespace amelia {

bool is_value_binding_node_type(NodeType node_type) {
  return node_type == NodeType::LetDeclNode || node_type == NodeType::ConstDeclNode ||
         node_type == NodeType::FunctionDeclNode;
}

bool is_type_binding_node_type(NodeType node_type) {
  // TODO
  return node_type == NodeType::TypeDeclNode;
}

Flex<Expression> build_expr_type_decl(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  const auto &type_decl_node = module_state.get_node(expr_node_id).as_TypeDeclNode();
  const auto &type_name_node = module_state.get_node(type_decl_node.name).as_IdentifierNode();

  auto binding = emplace_flex<TypeBinding>();
  binding->decl = expr_node_id;
  binding->name = type_name_node.name;
  binding->kind = BindingKind::Type;
  binding->visibility = DeclarationVisibility::Default;
  module_state.push_binding(binding);

  module_state.analyze_binding(binding);

  auto result = emplace_flex<TypeBindingStatement>();
  result->name = binding->name;
  result->binding = binding;
  result->body = build_expr_seq(module_state, expr_node_id, stmts);
  result->type = result->body->type;

  module_state.pop_binding();

  return result;
}

Flex<Expression> build_expr_type_binding(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  const Node &node = module_state.get_node(expr_node_id);
  if (node.type() == NodeType::TypeDeclNode) {
    return build_expr_type_decl(module_state, expr_node_id, stmts);
  } else {
    module_state.raise_error_at_node(
        expr_node_id, "not implemented (unknown node type in build_expr_type_binding)"
    );
  }
}

Flex<Expression> build_expr_fun_decl(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  const auto &fun_decl_node = module_state.get_node(expr_node_id).as_FunctionDeclNode();
  const auto &fun_name_node = module_state.get_node(fun_decl_node.name).as_IdentifierNode();

  size_t prior_bindings_size = module_state.get_binding_stack_size();

  auto binding = emplace_flex<ValueBinding>();
  binding->decl = expr_node_id;
  binding->name = fun_name_node.name;
  binding->kind = BindingKind::Function;
  binding->visibility = DeclarationVisibility::Default;
  module_state.push_binding(binding);

  // Swallow any subsequent function declarations with the same name into the same overload set
  while (true) {
    if (stmts.size() == 0) {
      break;
    }

    const auto &next_stmt_node = module_state.get_node(stmts[0]);
    if (next_stmt_node.type() != NodeType::FunctionDeclNode) {
      break;
    }

    const auto &next_fun_decl_node = next_stmt_node.as_FunctionDeclNode();
    const auto &next_fun_name_node = module_state.get_node(next_fun_decl_node.name)
                                         .as_IdentifierNode();
    if (next_fun_name_node.name != binding->name) {
      break;
    }

    auto next_fun_binding = emplace_flex<ValueBinding>();
    next_fun_binding->decl = stmts[0];
    next_fun_binding->name = next_fun_name_node.name;
    next_fun_binding->kind = BindingKind::Function;
    next_fun_binding->visibility = DeclarationVisibility::Default;
    module_state.push_binding(next_fun_binding);
    binding = next_fun_binding;
    stmts = ConstSlice<NodeId>(stmts.ptr() + 1, stmts.size() - 1);
  }

  module_state.analyze_binding(binding);

  auto result = emplace_flex<ValueBindingStatement>();
  result->binding = binding;
  result->body = build_expr_seq(module_state, expr_node_id, stmts);
  result->type = result->body->type;

  while (module_state.get_binding_stack_size() > prior_bindings_size) {
    module_state.pop_binding();
  }

  return result;
}

Flex<Expression> build_expr_var_decl(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  const Node &node = module_state.get_node(expr_node_id);
  NodeId target;
  Option<NodeId> type;
  Option<NodeId> expr;
  bool is_const;
  if (node.type() == NodeType::LetDeclNode) {
    const LetDeclNode &let_decl_node = node.as_LetDeclNode();
    target = let_decl_node.target;
    type = let_decl_node.type;
    expr = let_decl_node.expr;
    is_const = false;
  } else {
    const ConstDeclNode &const_decl_node = node.as_ConstDeclNode();
    target = const_decl_node.target;
    type = const_decl_node.type;
    expr = const_decl_node.expr;
    is_const = true;
  }
  auto binding = emplace_flex<ValueBinding>();
  binding->decl = expr_node_id;
  binding->name = module_state.get_node(target).as_IdentifierNode().name;
  binding->kind = is_const ? BindingKind::Constant : BindingKind::Variable;
  binding->visibility = DeclarationVisibility::Default;

  if (type.has_value()) {
    binding->type = evaluate_type_expr(module_state, type.value());
    if (expr.has_value()) {
      binding->value = expect_expression_of_type(module_state, binding->type.value(), expr.value());
    }
  } else {
    binding->type = UNKNOWN_TYPE;
    if (expr.has_value()) {
      binding->value = build_expression(module_state, expr.value());
      binding->type = is_const ? binding->value.value()->type
                               : binding->value.value()->type->remove_comptime_const_if_needed();
    }
  }

  auto result = emplace_flex<ValueBindingStatement>();
  result->binding = binding;
  module_state.push_binding(move(binding));
  result->body = build_expr_seq(module_state, expr_node_id, stmts);
  result->type = result->body->type;
  module_state.pop_binding();
  return result;
}

Flex<Expression> build_expr_value_binding(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  const Node &node = module_state.get_node(expr_node_id);
  if (node.type() == NodeType::LetDeclNode || node.type() == NodeType::ConstDeclNode) {
    return build_expr_var_decl(module_state, expr_node_id, stmts);
  } else if (node.type() == NodeType::FunctionDeclNode) {
    return build_expr_fun_decl(module_state, expr_node_id, stmts);
  } else {
    module_state.raise_error_at_node(
        expr_node_id, "not implemented (unknown node type in build_expr_value_binding)"
    );
  }
}

Flex<Expression> build_expr_seq(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  auto result = emplace_flex<StatementSequence>();
  result->type = NULL_TYPE;
  result->node_id = expr_node_id;
  for (size_t expr_index = 0; expr_index < stmts.size(); ++expr_index) {
    const auto &expr_node = module_state.get_node(stmts[expr_index]);
    if (is_value_binding_node_type(expr_node.type())) {
      auto expr = build_expr_value_binding(
          module_state, stmts[expr_index], SliceUtils::tail(stmts, expr_index + 1)
      );
      result->type = expr->type;
      result->stmts.push_back(expr);
      break;
    } else if (is_type_binding_node_type(expr_node.type())) {
      auto expr = build_expr_type_binding(
          module_state, stmts[expr_index], SliceUtils::tail(stmts, expr_index + 1)
      );
      result->type = expr->type;
      result->stmts.push_back(expr);
      break;
    }

    auto expr = build_expression(module_state, stmts[expr_index]);
    result->stmts.push_back(expr);
    if (expr_index == stmts.size() - 1 && !is_never_type(result->type)) {
      result->type = expr->type;
    }
  }
  return result;
}

Flex<Expression> build_block_expression(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const BlockExprNode &block_expr_node = module_state.get_node(expr_node_id).as_BlockExprNode();
  return build_expr_seq(module_state, expr_node_id, block_expr_node.stmts.data());
}

} // namespace amelia
