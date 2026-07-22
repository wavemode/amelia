#include "sequence_exprs.hpp"

#include "binding/data/type_binding.hpp"
#include "binding/data/value_binding.hpp"
#include "binding/logic/analysis.hpp"
#include "builtin/data/builtin_type.hpp"
#include "const/data/const_boolean_type.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "function/data/function_definition.hpp"
#include "literal/data/identifier_expression.hpp"
#include "literal/data/null_literal_expression.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "parser/data/node.hpp"
#include "sema/data/module_analysis_context.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "statement/data/break_statement.hpp"
#include "statement/data/continue_statement.hpp"
#include "statement/data/empty_statement.hpp"
#include "statement/data/goto_request.hpp"
#include "statement/data/goto_statement.hpp"
#include "statement/data/if_statement.hpp"
#include "statement/data/implicit_value_binding_statement.hpp"
#include "statement/data/label_statement.hpp"
#include "statement/data/return_statement.hpp"
#include "statement/data/statement_sequence.hpp"
#include "statement/data/switch_case_clause.hpp"
#include "statement/data/switch_statement.hpp"
#include "statement/data/switch_when_clause.hpp"
#include "statement/data/type_binding_statement.hpp"
#include "statement/data/value_binding_statement.hpp"
#include "statement/data/while_statement.hpp"
#include "type/logic/analysis.hpp"
#include "type/logic/type_conversion.hpp"
#include "util/data/flex.hpp"
#include "util/data/slice_utils.hpp"

namespace amelia {

namespace {

Flex<Expression> build_statement(IModuleAnalysisState &module_state, NodeId expr_node_id);

uint32_t push_label_scope(IModuleAnalysisState &module_state) {
  auto &ctx = module_state.analysis_context();
  uint32_t old_scope_level = ctx.current_scope_level;
  ctx.current_scope_level = ++ctx.max_scope_level;
  return old_scope_level;
}

void restore_label_scope(IModuleAnalysisState &module_state, uint32_t old_scope_level) {
  auto &ctx = module_state.analysis_context();
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
  auto &ctx = module_state.analysis_context();
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
  auto &ctx = module_state.analysis_context();
  if (!ctx.current_function_signature.has_value()) {
    module_state.raise_type_error_at_node(
        return_value->node_id, "Return value not within function"
    );
  }

  if (is_unknown_type(ctx.current_function_signature.value()->return_type)) {
    ctx.current_function_signature.value()->return_type = return_value->type
                                                              ->remove_comptime_const_from_type();
    return return_value;
  }

  return require_coerce(
      module_state,
      ctx.current_function_signature.value()->return_type,
      return_value,
      "Cannot convert expression of type '{1}' to expected return type '{2}'"
  );
}

Flex<Expression> build_expression_statement(
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

Flex<Expression> build_stmt_implicit_var_decl(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const auto &with_node = module_state.get_node(expr_node_id).as_WithStmtNode();

  auto result = emplace_flex<ImplicitValueBindingStatement>();
  result->node_id = expr_node_id;

  auto prior_binding_size = module_state.get_binding_stack_size();
  for (NodeId arg : with_node.args) {
    const auto &arg_node = module_state.get_node(arg).as_FunctionArgumentNode();
    if (!arg_node.name.has_value()) {
      module_state.raise_type_error_at_node(arg, "Implicit variable declarations must have a name");
    }
    const auto &arg_name = module_state.get_node(arg_node.name.value()).as_IdentifierNode();
    auto binding = emplace_flex<ValueBinding>();
    binding->decl = arg;
    binding->name = arg_name.name;
    binding->kind = BindingKind::Variable;
    binding->is_implicit = true;
    auto arg_value = build_expression(module_state, arg_node.expr);
    binding->value = arg_value;
    binding->type = arg_value->type;
    result->bindings.push_back(binding);
    module_state.push_binding(move(binding));
  }

  result->body = build_statement(module_state, with_node.body);
  result->type = result->body->type;

  while (module_state.get_binding_stack_size() > prior_binding_size) {
    module_state.pop_binding();
  }
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

Flex<Expression> build_return_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
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

Option<Flex<Expression>> build_stmt_with_introductory_decls(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> introductory_decls
) {
  if (introductory_decls.size() == 0) {
    return None();
  }

  auto &ctx = module_state.analysis_context();
  auto intro_decls_currently_analyzing = ctx.intro_decls_currently_analyzing;
  if (intro_decls_currently_analyzing.has_value() &&
      intro_decls_currently_analyzing.value() == expr_node_id) {
    return None();
  }
  List<NodeId> decls;
  for (size_t i = 0; i < introductory_decls.size(); ++i) {
    const Node &n = module_state.get_node(introductory_decls[i]);
    if (n.type() == NodeType::EmptyStmtNode) {
      continue;
    }
    decls.push_back(introductory_decls[i]);
  }
  decls.push_back(expr_node_id);
  ctx.intro_decls_currently_analyzing = expr_node_id;
  auto result = build_stmt_binding(module_state, decls[0], decls.data() + 1);
  ctx.intro_decls_currently_analyzing = intro_decls_currently_analyzing;
  return result;
}

Flex<Expression> build_while_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &while_node = module_state.get_node(expr_node_id).as_WhileStmtNode();
  auto &ctx = module_state.analysis_context();

  auto intro = build_stmt_with_introductory_decls(
      module_state, expr_node_id, while_node.introductory_decls.data()
  );
  if (intro.has_value()) {
    return intro.value();
  }

  auto result = emplace_flex<WhileStatement>();
  result->node_id = expr_node_id;
  result->type = NULL_TYPE;
  result->condition = expect_expression_of_type(module_state, BOOL_TYPE, while_node.condition);

  auto old_scope = push_label_scope(module_state);
  auto loop_currently_analyzing = ctx.loop_currently_analyzing.replace(expr_node_id);
  result->body = build_statement(module_state, while_node.body);
  ctx.loop_currently_analyzing = loop_currently_analyzing;
  restore_label_scope(module_state, old_scope);

  return result;
}

Flex<Expression> build_if_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &if_node = module_state.get_node(expr_node_id).as_IfStmtNode();
  auto &ctx = module_state.analysis_context();

  auto intro = build_stmt_with_introductory_decls(
      module_state, expr_node_id, if_node.introductory_decls.data()
  );
  if (intro.has_value()) {
    return intro.value();
  }

  auto result = emplace_flex<IfStatement>();
  result->node_id = expr_node_id;
  result->type = NULL_TYPE;
  result->condition = expect_expression_of_type(module_state, BOOL_TYPE, if_node.condition);

  auto old_scope = push_label_scope(module_state);
  result->then_branch = build_statement(module_state, if_node.then_branch);
  if (if_node.else_branch.has_value()) {
    result->else_branch = build_statement(module_state, if_node.else_branch.value());
  }
  restore_label_scope(module_state, old_scope);

  if (ctx.require_value_of_stmt) {
    if (result->else_branch.has_value()) {
      if (!require_branches_to_have_same_type(
              result->type, result->then_branch, result->else_branch.value()
          )) {
        String error_message("Cannot coerce expression of type '");
        result->else_branch.value()->type->remove_comptime_const_from_type()->serialize().to_string(
            error_message
        );
        error_message.append("' to expected type '");
        result->then_branch->type->remove_comptime_const_from_type()->serialize().to_string(
            error_message
        );
        error_message.append("' in else branch of if-statement in expression position");
        module_state.raise_type_error_at_node(expr_node_id, move(error_message));
      }
    } else {
      module_state.raise_type_error_at_node(
          expr_node_id, "Missing else branch in if-statement in expression position"
      );
    }
  }

  return result;
}

Flex<Expression> coerce_switch_body(
    IModuleAnalysisState &module_state, Option<Flex<Type>> expected_type, Flex<Expression> body_expr
) {
  if (expected_type.has_value()) {
    auto result = expected_type.value()->resolve_type()->coerce_expr(body_expr);
    if (!result.has_value()) {
      result = expected_type.value()
                   ->resolve_type()
                   ->remove_comptime_const_from_type()
                   ->coerce_expr(body_expr);
    }
    if (!result.has_value()) {
      String error_message("Cannot coerce expression of type '");
      body_expr->type->remove_comptime_const_from_type()->serialize().to_string(error_message);
      error_message.append("' to expected type '");
      expected_type.value()->remove_comptime_const_from_type()->serialize().to_string(error_message
      );
      error_message.append(" in body of switch-statement in expression position");
      module_state.raise_type_error_at_node(body_expr->node_id, move(error_message));
    }
    return result.value();
  } else {
    return body_expr;
  }
}

Flex<Expression> build_switch_when_clause(IModuleAnalysisState &module_state, NodeId when_node_id) {
  const auto &when_clause_node = module_state.get_node(when_node_id).as_WhenClauseNode();

  auto intro = build_stmt_with_introductory_decls(
      module_state, when_node_id, when_clause_node.introductory_decls.data()
  );
  if (intro.has_value()) {
    return intro.value();
  }

  auto &ctx = module_state.analysis_context();

  auto condition_expr = build_expression(module_state, when_clause_node.condition);
  auto condition = require_boolean_expr(condition_expr);
  if (!condition.has_value()) {
    String error_message("Condition expression of type '");
    condition_expr->type->serialize().to_string(error_message);
    error_message.append("' cannot be coerced to bool");
    module_state.raise_type_error_at_node(when_node_id, move(error_message));
  }

  auto body_stmt = coerce_switch_body(
      module_state,
      ctx.switch_case_expected_type,
      build_statement(module_state, ctx.switch_case_body_stmt_node_id.value())
  );
  auto result = emplace_flex<SwitchWhenClause>();
  result->node_id = when_node_id;
  result->condition = condition.value();
  result->body = body_stmt;
  result->type = body_stmt->type;
  return result;
}

void add_value_to_case_clause_condition(
    IModuleAnalysisState &module_state,
    Flex<Expression> &condition,
    const Expression &subject_expr,
    Flex<Expression> value_expr
) {
  auto equality_expr = perform_binary_op(
      value_expr->node_id, BinaryOperatorKind::Equals, subject_expr, value_expr
  );
  if (!equality_expr.has_value()) {
    String error_message("Cannot compare expression of type '");
    subject_expr.type->serialize().to_string(error_message);
    error_message.append("' to expression of type '");
    value_expr->type->serialize().to_string(error_message);
    error_message.append("' in case clause of switch-statement");
    module_state.raise_type_error_at_node(value_expr->node_id, move(error_message));
  }

  equality_expr = require_boolean_expr(equality_expr.value());
  if (!equality_expr.has_value()) {
    String error_message("Equality comparison of '");
    subject_expr.type->serialize().to_string(error_message);
    error_message.append("' and '");
    value_expr->type->serialize().to_string(error_message);
    error_message.append("' does not produce a boolean result");
  }

  if (condition.is_null()) {
    condition = equality_expr.value();
  } else {
    condition = perform_binary_op(
                    value_expr->node_id, BinaryOperatorKind::Or, condition, equality_expr.value()
    )
                    .value();
  }
}

Flex<Expression> build_switch_case_clause(IModuleAnalysisState &module_state, NodeId case_node_id) {
  const auto &case_clause_node = module_state.get_node(case_node_id).as_CaseClauseNode();
  const auto &case_clause_header_node = module_state.get_node(case_clause_node.header)
                                            .as_CaseClauseHeaderNode();

  if (case_clause_header_node.introductory_decls.has_value()) {
    auto intro = build_stmt_with_introductory_decls(
        module_state, case_node_id, case_clause_header_node.introductory_decls.value().data()
    );
    if (intro.has_value()) {
      return intro.value();
    }
  }

  auto &ctx = module_state.analysis_context();

  auto result = emplace_flex<SwitchCaseClause>();
  result->node_id = case_node_id;

  if (case_clause_header_node.exprs.has_value()) {
    Flex<Expression> condition;
    for (NodeId value_node_id : case_clause_header_node.exprs.value()) {
      auto value_expr = build_expression(module_state, value_node_id);
      add_value_to_case_clause_condition(
          module_state, condition, ctx.switch_subject_expr.value(), value_expr
      );
      if (ctx.switch_case_value_exprs.has_value()) {
        ctx.switch_case_value_exprs.value()->push_back(value_expr);
      }
    }
    if (!condition.is_null()) {
      result->condition = condition;
    }
  }

  auto old_scope = push_label_scope(module_state);
  auto old_switch_case_body_stmt_node_id = ctx.switch_case_body_stmt_node_id;
  ctx.switch_case_body_stmt_node_id = case_clause_node.body;

  if (case_clause_header_node.when_clause.has_value()) {
    auto when_clause_expr = build_switch_when_clause(
        module_state, case_clause_header_node.when_clause.value()
    );
    result->when_body = when_clause_expr;
    result->type = when_clause_expr->type;
  } else {
    auto body_stmt = coerce_switch_body(
        module_state,
        ctx.switch_case_expected_type,
        build_statement(module_state, case_clause_node.body)
    );
    result->expr_body = body_stmt;
    result->type = body_stmt->type;
  }

  ctx.switch_case_body_stmt_node_id = old_switch_case_body_stmt_node_id;
  restore_label_scope(module_state, old_scope);

  return result;
}

bool are_case_values_exhaustive(
    const Expression &subject_expr, ConstSlice<Flex<Expression>> value_expressions
) {
  if (is_bool_type(subject_expr.type)) {
    bool has_true = false;
    bool has_false = false;
    for (const auto &value_expr : value_expressions) {
      if (value_expr->type->is<ConstBooleanType>()) {
        if (value_expr->type->as<ConstBooleanType>().value) {
          has_true = true;
        } else {
          has_false = true;
        }
      }
    }
    return has_true && has_false;
  }

  // TODO: enums, sealed type tags, tuples

  return false;
}

Flex<Expression> build_switch_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  auto &switch_node = module_state.get_node(expr_node_id).as_SwitchStmtNode();

  auto intro = build_stmt_with_introductory_decls(
      module_state, expr_node_id, switch_node.introductory_decls.data()
  );
  if (intro.has_value()) {
    return intro.value();
  }

  auto &ctx = module_state.analysis_context();

  List<Flex<Expression>> case_values;

  auto old_switch_case_expected_type = ctx.switch_case_expected_type;
  auto old_switch_case_value_exprs = ctx.switch_case_value_exprs;
  auto old_switch_subject_expr = ctx.switch_subject_expr;

  ctx.switch_case_expected_type = None();
  if (ctx.require_value_of_stmt) {
    ctx.switch_case_value_exprs = &case_values;
  }

  auto subject_expr = build_expression(module_state, switch_node.expr);
  auto subject_binding = emplace_flex<ValueBinding>();
  subject_binding->decl = expr_node_id;
  subject_binding->name = "aM_switch_subject";
  subject_binding->kind = BindingKind::Constant;
  subject_binding->visibility = DeclarationVisibility::Default;
  subject_binding->value = subject_expr;
  subject_binding->type = subject_expr->type;

  auto subject_var_expr = emplace_flex<IdentifierExpression>();
  subject_var_expr->node_id = switch_node.expr;
  subject_var_expr->type = subject_expr->type;
  subject_var_expr->binding = subject_binding;
  ctx.switch_subject_expr = subject_var_expr;

  auto result = emplace_flex<SwitchStatement>();
  result->node_id = expr_node_id;
  result->type = NULL_TYPE;

  for (NodeId case_clause_node_id : switch_node.clauses) {
    auto case_clause_expr = build_switch_case_clause(module_state, case_clause_node_id);
    result->case_clauses.push_back(case_clause_expr);
    if (ctx.require_value_of_stmt && !ctx.switch_case_expected_type.has_value()) {
      ctx.switch_case_expected_type = case_clause_expr->type;
      result->type = case_clause_expr->type;
    }
  }

  if (switch_node.default_body.has_value()) {
    result->default_body = coerce_switch_body(
        module_state,
        ctx.switch_case_expected_type,
        build_statement(module_state, switch_node.default_body.value())
    );
  } else if (ctx.require_value_of_stmt) {
    if (!are_case_values_exhaustive(
            ctx.switch_subject_expr.value(), ctx.switch_case_value_exprs.value()->data()
        )) {
      module_state.raise_type_error_at_node(
          expr_node_id,
          "Missing default clause in non-exhaustive switch-statement in expression position"
      );
    }
  }

  ctx.switch_case_expected_type = old_switch_case_expected_type;
  ctx.switch_case_value_exprs = old_switch_case_value_exprs;
  ctx.switch_subject_expr = old_switch_subject_expr;

  auto result_wrapper = emplace_flex<ValueBindingStatement>();
  result_wrapper->binding = subject_binding;
  result_wrapper->body = result;
  result_wrapper->type = result->type;
  return result_wrapper;
}

Flex<Expression> build_block_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const BlockStmtNode &block_expr_node = module_state.get_node(expr_node_id).as_BlockStmtNode();
  return build_stmt_seq(module_state, expr_node_id, block_expr_node.stmts.data());
}

Flex<Expression> build_label_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const LabelStmtNode &label_stmt_node = module_state.get_node(expr_node_id).as_LabelStmtNode();
  const IdentifierNode &label_name_node = module_state.get_node(label_stmt_node.label)
                                              .as_IdentifierNode();
  auto &ctx = module_state.analysis_context();
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
  auto &ctx = module_state.analysis_context();

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
  auto &ctx = module_state.analysis_context();
  if (!ctx.loop_currently_analyzing.has_value()) {
    module_state.raise_type_error_at_node(expr_node_id, "Break statement not within loop");
  }
  auto result = emplace_flex<BreakStatement>();
  result->node_id = expr_node_id;
  result->type = NEVER_TYPE;
  return result;
}

Flex<Expression> build_continue_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  auto &ctx = module_state.analysis_context();
  if (!ctx.loop_currently_analyzing.has_value()) {
    module_state.raise_type_error_at_node(expr_node_id, "Continue statement not within loop");
  }
  auto result = emplace_flex<ContinueStatement>();
  result->node_id = expr_node_id;
  result->type = NEVER_TYPE;
  return result;
}

Flex<Expression> build_statement(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &expr_node = module_state.get_node(expr_node_id);
  Flex<Expression> result;
  switch (expr_node.type()) {
  case NodeType::ExprStmtNode:
    result = build_expression_statement(module_state, expr_node_id);
    break;
  case NodeType::EmptyStmtNode:
    result = emplace_flex<EmptyStatement>();
    result->node_id = expr_node_id;
    result->type = NULL_TYPE;
    break;
  case NodeType::ReturnStmtNode:
    result = build_return_statement(module_state, expr_node_id);
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
    result = build_while_statement(module_state, expr_node_id);
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
  case NodeType::ContinueStmtNode:
    result = build_continue_statement(module_state, expr_node_id);
    break;
  case NodeType::IfStmtNode:
    result = build_if_statement(module_state, expr_node_id);
    break;
  case NodeType::SwitchStmtNode:
    result = build_switch_statement(module_state, expr_node_id);
    break;
  case NodeType::CaseClauseNode:
    result = build_switch_case_clause(module_state, expr_node_id);
    break;
  case NodeType::WhenClauseNode:
    result = build_switch_when_clause(module_state, expr_node_id);
    break;
  case NodeType::WithStmtNode:
    result = build_stmt_implicit_var_decl(module_state, expr_node_id);
    break;
  default:
    module_state.raise_type_error_at_node(
        expr_node_id, "not implemented (unknown node type in build_statement)"
    );
  }

  return result;
}

Flex<Expression> build_stmt_seq(
    IModuleAnalysisState &module_state,
    NodeId expr_node_id,
    ConstSlice<NodeId> stmts,
    bool require_value_of_last_stmt
) {
  auto &ctx = module_state.analysis_context();
  bool require_value_of_stmt = ctx.require_value_of_stmt;

  auto old_scope = push_label_scope(module_state);
  auto result = emplace_flex<StatementSequence>();
  result->type = NULL_TYPE;
  result->node_id = expr_node_id;
  for (size_t expr_index = 0; expr_index < stmts.size(); ++expr_index) {
    if (expr_index == stmts.size() - 1) {
      ctx.require_value_of_stmt = require_value_of_last_stmt;
    } else {
      ctx.require_value_of_stmt = false;
    }

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
  restore_label_scope(module_state, old_scope);
  ctx.require_value_of_stmt = require_value_of_stmt;
  return result;
}

} // namespace

Flex<Expression> build_stmt_seq(
    IModuleAnalysisState &module_state, NodeId expr_node_id, ConstSlice<NodeId> stmts
) {
  auto &ctx = module_state.analysis_context();
  return build_stmt_seq(module_state, expr_node_id, stmts, ctx.require_value_of_stmt);
}

Flex<Expression> build_block_expression(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const BlockExprNode &block_expr_node = module_state.get_node(expr_node_id).as_BlockExprNode();
  auto result = build_stmt_seq(module_state, expr_node_id, block_expr_node.stmts.data(), true);
  auto &stmt_seq = result->as<StatementSequence>();
  if (stmt_seq.stmts.size() > 0) {
  }
  return result;
}

} // namespace amelia
