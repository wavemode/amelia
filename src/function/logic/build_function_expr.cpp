#include "build_function_expr.hpp"

#include "binding/data/value_binding.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "function/data/function_call_expression.hpp"
#include "function/data/function_type.hpp"
#include "literal/data/identifier_expression.hpp"
#include "literal/logic/build_literal_expr.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "parser/data/node.hpp"
#include "sema/data/module_analysis_context.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "statement/logic/sequence_exprs.hpp"
#include "util/data/flex.hpp"
#include "util/data/map.hpp"

namespace amelia {

Flex<Expression> build_funcall_expression(IModuleAnalysisState &module_state, NodeId expr_node_id) {
  const auto &call_node = module_state.get_node(expr_node_id).as_FunctionCallExprNode();
  auto callee_expr = build_expression(module_state, call_node.callee);
  List<Flex<Expression>> pos_args;
  Map<Text, Flex<Expression>> named_args;
  for (const auto &arg_node_id : call_node.args) {
    const auto &arg_node = module_state.get_node(arg_node_id).as_FunctionArgumentNode();
    auto expr = build_expression(module_state, arg_node.expr);
    if (arg_node.name.has_value()) {
      const auto &name_node = module_state.get_node(arg_node.name.value()).as_IdentifierNode();
      if (named_args.has(name_node.name)) {
        String error_message = "Duplicate argument name '";
        error_message.append(name_node.name);
        error_message.append("' in function call");
        module_state.raise_type_error_at_node(arg_node.name.value(), move(error_message));
      }
      named_args.set(name_node.name, move(expr));
    } else {
      if (named_args.size() > 0) {
        module_state.raise_type_error_at_node(
            arg_node_id, "Positional arguments must appear before named arguments"
        );
      }
      pos_args.push_back(expr);
    }
  }
  auto result = resolve_function_call(
      module_state, expr_node_id, callee_expr, pos_args.data(), named_args
  );
  if (!result.has_value()) {
    String error_message = "No function for call to '";
    error_message.append(static_cast<const FunctionType &>(*callee_expr->type).name);
    error_message.append("' matches the given arguments (");
    for (size_t i = 0; i < pos_args.size(); ++i) {
      if (i > 0) {
        error_message.append(", ");
      }
      pos_args[i]->type->serialize().to_string(error_message);
    }
    size_t named_arg_count = 0;
    for (const auto &named_arg : named_args) {
      if (named_arg_count > 0 || pos_args.size() > 0) {
        error_message.append(", ");
      }
      error_message.append(named_arg.first);
      error_message.append(": ");
      named_arg.second->type->serialize().to_string(error_message);
      ++named_arg_count;
    }
    error_message.append(")");
    module_state.raise_type_error_at_node(expr_node_id, move(error_message));
  }
  return result.value();
}

Option<Flex<FunctionCallExpression>> resolve_function_call(
    IModuleAnalysisState &module_state,
    NodeId expr_node_id,
    Flex<Expression> callee,
    Slice<Flex<Expression>> pos_args,
    const Map<Text, Flex<Expression>> &named_args
) {
  auto &ctx = module_state.analysis_context();

  if (!callee->type->is<FunctionType>()) {
    module_state.raise_type_error_at_node(
        expr_node_id, "not implemented (called expression is not a function)"
    );
  }

  // First, we try to find a signature that exactly matches the types we passed. Then, we jump
  // back to start and look for the first signature (in source declaration order) that is
  // callable via implicit conversion of the passed args.
  // TODO: do this in a single pass
  bool exact_match_only = true;
start:

  List<Option<Flex<Expression>>> arguments;
  Map<Text, Option<Flex<Expression>>> implicit_arguments;
  size_t signature_id = 0;
  for (FunctionDefinition &definition : callee->type.downcast<FunctionType>()->definitions) {
    auto &signature = *definition.signature;
    arguments.clear();
    implicit_arguments.clear();

    size_t pos_arg_index = 0;
    size_t used_named_args = 0;
    for (FunctionParameter &param : signature.parameters) {
      if (pos_arg_index < pos_args.size()) {
        Option<Flex<Expression>> expr;
        if (exact_match_only) {
          expr = param.type->unify_type(pos_args[pos_arg_index]->type) ? pos_args[pos_arg_index]
                                                                       : Option<Flex<Expression>>();
        } else {
          expr = param.type->coerce_expr(pos_args[pos_arg_index]);
        }
        if (!expr.has_value()) {
          goto fail;
        }
        arguments.push_back(expr);
        ++pos_arg_index;
      } else if (named_args.has(param.name)) {
        Option<Flex<Expression>> expr;
        if (exact_match_only) {
          auto arg_expr = named_args[param.name];
          expr = param.type->unify_type(arg_expr->type) ? arg_expr : Option<Flex<Expression>>();
        } else {
          auto arg_expr = named_args[param.name];
          expr = param.type->coerce_expr(arg_expr);
        }
        if (!expr.has_value()) {
          goto fail;
        }
        arguments.push_back(expr);
        ++used_named_args;
      } else if (param.default_value.has_value()) {
        arguments.push_back(None());
      } else {
        goto fail;
      }
    }

    if (signature.implicit_parameters.has_value()) {
      for (FunctionParameter &param : signature.implicit_parameters.value()) {
        if (named_args.has(param.name)) {
          Option<Flex<Expression>> expr;
          if (exact_match_only) {
            auto arg_expr = named_args[param.name];
            expr = param.type->unify_type(arg_expr->type) ? arg_expr : Option<Flex<Expression>>();
          } else {
            auto arg_expr = named_args[param.name];
            expr = param.type->coerce_expr(arg_expr);
          }
          if (!expr.has_value()) {
            goto fail;
          }
          implicit_arguments.set(param.name, expr.value());
          ++used_named_args;
        }
      }
    }

    if (pos_arg_index < pos_args.size() || used_named_args < named_args.size()) {
      goto fail;
    }

    {
      auto result = emplace_flex<FunctionCallExpression>();
      result->node_id = expr_node_id;
      result->type = signature.return_type.weak();
      result->callee = callee;
      result->signature_id = signature_id;
      result->arguments = move(arguments);

      if (signature.implicit_parameters.has_value()) {
        for (FunctionParameter &param : signature.implicit_parameters.value()) {

          // check if implicit param was already passed explicitly as named argument
          if (implicit_arguments.has(param.name)) {
            continue;
          }

          // check for implicit param in current implicit scope
          auto implicit_binding_id = module_state.get_implicit_binding_id_by_name(param.name);
          if (implicit_binding_id.has_value()) {
            auto implicit_binding = module_state.get_binding_by_id(implicit_binding_id.value());
            auto arg_expr = emplace_flex<IdentifierExpression>();
            arg_expr->node_id = expr_node_id;
            arg_expr->type = implicit_binding.downcast<ValueBinding>()->type.value().weak();
            arg_expr->binding = implicit_binding;

            auto coerced_arg_expr = param.type->coerce_expr(arg_expr);
            if (!coerced_arg_expr.has_value()) {
              String error_message = "Cannot coerce implicit argument '";
              error_message.append(param.name);
              error_message.append("' from type '");
              arg_expr->type->serialize().to_string(error_message);
              error_message.append("' to expected type '");
              param.type->serialize().to_string(error_message);
              error_message.append("' in call to function '");
              error_message.append(callee->type.downcast<FunctionType>()->name);
              error_message.append("'");
              module_state.raise_type_error_at_node(expr_node_id, move(error_message));
            }

            implicit_arguments.set(param.name, coerced_arg_expr.value());
            continue;
          }

          // check if we can add implicit param to function currently being analyzed
          if (ctx.current_inferred_implicit_params.has_value()) {
            auto arg_expr_binding = ctx.current_inferred_implicit_params.value()->find(param.name);
            if (arg_expr_binding.has_value()) {
              auto arg_expr = emplace_flex<IdentifierExpression>();
              arg_expr->node_id = expr_node_id;
              arg_expr->binding = *arg_expr_binding.value();
              arg_expr->type = (*arg_expr_binding.value())->type.value().weak();
              auto coerced_arg_expr = param.type->coerce_expr(arg_expr);
              if (!coerced_arg_expr.has_value()) {
                String error_message = "Cannot coerce implicit argument '";
                error_message.append(param.name);
                error_message.append("' from inferred type '");
                arg_expr->type->serialize().to_string(error_message);
                error_message.append("' to expected type '");
                param.type->serialize().to_string(error_message);
                error_message.append("' in call to function '");
                error_message.append(callee->type.downcast<FunctionType>()->name);
                error_message.append("'");
                module_state.raise_type_error_at_node(expr_node_id, move(error_message));
              }
              implicit_arguments.set(param.name, coerced_arg_expr.value());
            } else {
              auto binding = emplace_flex<ValueBinding>();
              binding->decl = expr_node_id;
              binding->name = param.name;
              binding->type = param.type.weak();
              binding->kind = BindingKind::Variable;
              binding->is_implicit = true;
              ctx.current_inferred_implicit_params.value()->set(param.name, binding);

              auto arg_expr = emplace_flex<IdentifierExpression>();
              arg_expr->node_id = expr_node_id;
              arg_expr->binding = binding;
              arg_expr->type = binding->type.value().weak();
              implicit_arguments.set(param.name, arg_expr);
            }
            continue;
          }

          // check if the implicit param has a default value
          if (param.default_value.has_value()) {
            implicit_arguments.set(param.name, None());
            continue;
          }

          String error_message = "Missing implicit argument '";
          error_message.append(param.name);
          error_message.append("' of type '");
          param.type->serialize().to_string(error_message);
          error_message.append("' in call to function '");
          error_message.append(callee->type.downcast<FunctionType>()->name);
          error_message.append("'");
          module_state.raise_type_error_at_node(expr_node_id, move(error_message));
        }
      }

      result->implicit_arguments = move(implicit_arguments);
      return result;
    }

  fail:
    ++signature_id;
  }

  if (exact_match_only) {
    exact_match_only = false;
    goto start;
  }

  return None();
}

} // namespace amelia
