#include "sequence_exprs.hpp"

#include "binding/data/type_binding.hpp"
#include "binding/data/value_binding.hpp"
#include "binding/logic/analysis.hpp"
#include "builtin/data/builtin_type.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "function/data/function_signature.hpp"
#include "literal/data/null_literal_expression.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "parser/data/node.hpp"
#include "sema/data/module_analysis_context.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "statement/data/empty_statement.hpp"
#include "statement/data/goto_request.hpp"
#include "statement/data/goto_statement.hpp"
#include "statement/data/label_statement.hpp"
#include "statement/data/return_statement.hpp"
#include "statement/data/statement_sequence.hpp"
#include "statement/data/type_binding_statement.hpp"
#include "statement/data/value_binding_statement.hpp"
#include "statement/data/break_statement.hpp"
#include "statement/data/while_statement.hpp"
#include "type/logic/analysis.hpp"
#include "util/data/flex.hpp"
#include "util/data/slice_utils.hpp"

namespace amelia {

namespace {

Flex<Expression> build_statement(IModuleAnalysisState &module_state, NodeId expr_node_id);

uint32_t push_scope(IModuleAnalysisState &module_state) {
  auto &ctx = module_state.current_context();
  uint32_t old_scope_level = ctx.current_scope_level;
  ctx.current_scope_level = ++ctx.max_scope_level;
  return old_scope_level;
}

void restore_scope(IModuleAnalysisState &module_state, uint32_t old_scope_level) {
  auto &ctx = module_state.current_context();
  ctx.current_scope_level = old_scope_level;
  if (old_scope_level == 0) {
    if (ctx.gotos_in_scope.size() > 0) {
      for (const auto &[label_name, goto_request] : ctx.gotos_in_scope) {
        String error_message = "Label '";
        error_message.append(label_name);
        error_message.append("' not found in scope");
        module_state.raise_type_error_at_node(goto_request.goto_stmt->node_id, move(error_message));
      }
    }
    ctx.labels_in_scope.clear();
  }
}

void perform_goto(IModuleAnalysisState &module_state, Text label_name) {
  auto &ctx = module_state.current_context();
  auto label_scope_level = ctx.labels_in_scope.get(label_name);
  const auto &goto_request = ctx.gotos_in_scope.get(label_name);

  if (goto_request.goto_scope_level < label_scope_level) {
    module_state.raise_type_error_at_node(
        goto_request.goto_stmt->node_id, "This goto would illegally enter a new scope"
    );
  }

  ctx.gotos_in_scope.remove(label_name);
}

bool is_binding_node_type(NodeType node_type) {
  return node_type == NodeType::LetDeclNode || node_type == NodeType::ConstDeclNode ||
         node_type == NodeType::FunctionDeclNode || node_type == NodeType::TypeDeclNode;
}

Flex<Expression> assign_current_function_return_value(
    IModuleAnalysisState &module_state, Flex<Expression> return_value
) {
  if (!module_state.current_context().current_function_signature.has_value()) {
    module_state.raise_type_error_at_node(
        return_value->node_id, "Return value not within function"
    );
  }

  if (is_unknown_type(module_state.current_context().current_function_signature.value()->return_type
      )) {
    module_state.current_context()
        .current_function_signature.value()
        ->return_type = return_value->type->remove_comptime_const_from_type();
    return return_value;
  }

  return require_coerce(
      module_state,
      module_state.current_context().current_function_signature.value()->return_type,
      return_value,
      "Cannot convert expression of type '{1}' to expected return type '{2}'"
  );
}

Flex<Expression> build_stmt_expression_statement(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const auto &expr_stmt_node = module_state.get_node(expr_node_id).as_ExprStmtNode();
  return build_expression(module_state, expr_stmt_node.expr);
}

Flex<Expression> build_stmt_type_decl(
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

  analyze_binding(module_state, binding);

  auto result = emplace_flex<TypeBindingStatement>();
  result->name = binding->name;
  result->binding = binding;
  result->body = build_stmt_seq(module_state, expr_node_id, stmts);
  result->type = result->body->type;

  module_state.pop_binding();

  return result;
}

Flex<Expression> build_stmt_fun_decl(
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

  analyze_binding(module_state, binding);

  auto result = emplace_flex<ValueBindingStatement>();
  result->binding = binding;
  result->body = build_stmt_seq(module_state, expr_node_id, stmts);
  result->type = result->body->type;

  while (module_state.get_binding_stack_size() > prior_bindings_size) {
    module_state.pop_binding();
  }

  return result;
}

Flex<Expression> build_stmt_var_decl(
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
                               : binding->value.value()->type->remove_comptime_const_from_type();
    }
  }

  auto result = emplace_flex<ValueBindingStatement>();
  result->binding = binding;
  module_state.push_binding(move(binding));
  result->body = build_stmt_seq(module_state, expr_node_id, stmts);
  result->type = result->body->type;
  module_state.pop_binding();
  return result;
}

Flex<Expression> build_stmt_binding(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  const Node &node = module_state.get_node(expr_node_id);
  if (node.type() == NodeType::LetDeclNode || node.type() == NodeType::ConstDeclNode) {
    return build_stmt_var_decl(module_state, expr_node_id, stmts);
  } else if (node.type() == NodeType::FunctionDeclNode) {
    return build_stmt_fun_decl(module_state, expr_node_id, stmts);
  } else if (node.type() == NodeType::TypeDeclNode) {
    return build_stmt_type_decl(module_state, expr_node_id, stmts);
  } else {
    module_state.raise_type_error_at_node(
        expr_node_id, "not implemented (unknown node type in build_expr_binding)"
    );
  }
}

Flex<Expression> build_stmt_return(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &return_node = module_state.get_node(expr_node_id).as_ReturnStmtNode();
  auto result = emplace_flex<ReturnStatement>();
  result->node_id = expr_node_id;
  if (return_node.expr.has_value()) {
    result->value = assign_current_function_return_value(
        module_state, build_expression(module_state, return_node.expr.value())
    );
  } else {
    Flex<Expression> implied_return_value = emplace_flex<NullLiteralExpression>();
    implied_return_value->node_id = expr_node_id;
    implied_return_value->type = NULL_TYPE;
    result->value = assign_current_function_return_value(module_state, implied_return_value);
  }
  result->type = NEVER_TYPE;
  return result;
}

Flex<Expression> build_stmt_while(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &while_node = module_state.get_node(expr_node_id).as_WhileStmtNode();
  auto &ctx = module_state.current_context();

  if (while_node.introductory_decls.size() > 0) {
    auto intro_decls_currently_analyzing = ctx
                                               .intro_decls_currently_analyzing;
    if (!intro_decls_currently_analyzing.has_value() ||
        intro_decls_currently_analyzing.value() != expr_node_id) {
      List<NodeId> decls;
      for (size_t i = 0; i < while_node.introductory_decls.size(); ++i) {
        const Node &n = module_state.get_node(while_node.introductory_decls[i]);
        if (n.type() == NodeType::EmptyStmtNode) {
          continue;
        }
        decls.push_back(while_node.introductory_decls[i]);
      }
      decls.push_back(expr_node_id);
      ctx.intro_decls_currently_analyzing = expr_node_id;
      auto result = build_stmt_binding(module_state, decls[0], decls.data() + 1);
      ctx
          .intro_decls_currently_analyzing = intro_decls_currently_analyzing;
      return result;
    }
  }

  auto result = emplace_flex<WhileStatement>();
  result->node_id = expr_node_id;
  result->type = NULL_TYPE;
  result->condition = expect_expression_of_type(module_state, BOOL_TYPE, while_node.condition);

  auto old_scope = push_scope(module_state);
  auto loop_currently_analyzing = ctx.loop_currently_analyzing.replace(expr_node_id);
  result->body = build_statement(module_state, while_node.body);
  ctx.loop_currently_analyzing = loop_currently_analyzing;
  restore_scope(module_state, old_scope);

  return result;
}

Flex<Expression> build_block_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const BlockStmtNode &block_expr_node = module_state.get_node(expr_node_id).as_BlockStmtNode();
  return build_stmt_seq(module_state, expr_node_id, block_expr_node.stmts.data());
}

Flex<Expression> build_label_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const LabelStmtNode &label_stmt_node = module_state.get_node(expr_node_id).as_LabelStmtNode();
  const IdentifierNode &label_name_node = module_state.get_node(label_stmt_node.label)
                                              .as_IdentifierNode();
  auto &ctx = module_state.current_context();
  if (ctx.labels_in_scope.has(label_name_node.name)) {
    String error_message = "Duplicate label name '";
    error_message.append(label_name_node.name);
    error_message.append("' in scope");
    module_state.raise_type_error_at_node(expr_node_id, move(error_message));
  }

  ctx.labels_in_scope.set(label_name_node.name, ctx.current_scope_level);

  if (ctx.gotos_in_scope.has(label_name_node.name)) {
    perform_goto(module_state, label_name_node.name);
  }

  auto result = emplace_flex<LabelStatement>();
  result->node_id = expr_node_id;
  result->name = label_name_node.name;
  result->type = NULL_TYPE;
  return result;
}

Flex<Expression> build_goto_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const GotoStmtNode &goto_stmt_node = module_state.get_node(expr_node_id).as_GotoStmtNode();
  const IdentifierNode &label_name_node = module_state.get_node(goto_stmt_node.label)
                                              .as_IdentifierNode();
  auto &ctx = module_state.current_context();

  auto goto_statement = emplace_flex<GotoStatement>();
  goto_statement->node_id = expr_node_id;
  goto_statement->label = label_name_node.name;
  goto_statement->type = NEVER_TYPE;

  ctx.gotos_in_scope.set(label_name_node.name, {&*goto_statement, ctx.current_scope_level});

  if (ctx.labels_in_scope.has(label_name_node.name)) {
    perform_goto(module_state, label_name_node.name);
  }

  return goto_statement;
}

Flex<Expression> build_break_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  auto &ctx = module_state.current_context();
  if (!ctx.loop_currently_analyzing.has_value()) {
    module_state.raise_type_error_at_node(expr_node_id, "Break statement not within loop");
  }
  auto result = emplace_flex<BreakStatement>();
  result->node_id = expr_node_id;
  result->type = NEVER_TYPE;
  return result;
}

Flex<Expression> build_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &expr_node = module_state.get_node(expr_node_id);
  Flex<Expression> result;
  switch (expr_node.type()) {
  case NodeType::ExprStmtNode:
    result = build_stmt_expression_statement(module_state, expr_node_id);
    break;
  case NodeType::EmptyStmtNode:
    result = emplace_flex<EmptyStatement>();
    result->node_id = expr_node_id;
    result->type = NULL_TYPE;
    break;
  case NodeType::ReturnStmtNode:
    result = build_stmt_return(module_state, expr_node_id);
    break;
  case NodeType::PreDecrementStmtNode:
  case NodeType::PostDecrementStmtNode:
  case NodeType::PreIncrementStmtNode:
  case NodeType::PostIncrementStmtNode:
    result = build_unary_operator_expression(module_state, expr_node_id);
    break;
  case NodeType::AddAssignStmtNode:
  case NodeType::AssignmentStmtNode:
  case NodeType::BitwiseAndAssignStmtNode:
  case NodeType::BitwiseOrAssignStmtNode:
  case NodeType::BitwiseXorAssignStmtNode:
  case NodeType::DivAssignStmtNode:
  case NodeType::LeftShiftAssignStmtNode:
  case NodeType::ModAssignStmtNode:
  case NodeType::MulAssignStmtNode:
  case NodeType::RightShiftAssignStmtNode:
  case NodeType::SubAssignStmtNode:
    result = build_binary_operator_expression(module_state, expr_node_id);
    break;
  case NodeType::WhileStmtNode:
    result = build_stmt_while(module_state, expr_node_id);
    break;
  case NodeType::LetDeclNode:
  case NodeType::ConstDeclNode:
  case NodeType::FunctionDeclNode:
  case NodeType::TypeDeclNode:
    result = build_stmt_binding(module_state, expr_node_id, ConstSlice<NodeId>());
    break;
  case NodeType::BlockStmtNode:
    result = build_block_statement(module_state, expr_node_id);
    break;
  case NodeType::LabelStmtNode:
    result = build_label_statement(module_state, expr_node_id);
    break;
  case NodeType::GotoStmtNode:
    result = build_goto_statement(module_state, expr_node_id);
    break;
  case NodeType::BreakStmtNode:
    result = build_break_statement(module_state, expr_node_id);
    break;
  default:
    module_state.raise_type_error_at_node(
        expr_node_id, "not implemented (unknown node type in build_statement)"
    );
  }

  return result;
}

} // namespace

Flex<Expression> build_stmt_seq(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  auto old_scope = push_scope(module_state);
  auto result = emplace_flex<StatementSequence>();
  result->type = NULL_TYPE;
  result->node_id = expr_node_id;
  for (size_t expr_index = 0; expr_index < stmts.size(); ++expr_index) {
    const auto &expr_node = module_state.get_node(stmts[expr_index]);
    if (is_binding_node_type(expr_node.type())) {
      auto expr = build_stmt_binding(
          module_state, stmts[expr_index], SliceUtils::tail(stmts, expr_index + 1)
      );
      result->type = expr->type;
      result->stmts.push_back(expr);
      break;
    } else {
      auto expr = build_statement(module_state, stmts[expr_index]);
      result->stmts.push_back(expr);
      if (expr_index == stmts.size() - 1 && !is_never_type(result->type)) {
        result->type = expr->type;
      }
    }
  }
  restore_scope(module_state, old_scope);
  return result;
}

Flex<Expression> build_block_expression(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const BlockExprNode &block_expr_node = module_state.get_node(expr_node_id).as_BlockExprNode();
  return build_stmt_seq(module_state, expr_node_id, block_expr_node.stmts.data());
}

} // namespace amelia
