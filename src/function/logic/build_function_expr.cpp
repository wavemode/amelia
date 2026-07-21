#include "build_function_expr.hpp"

#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "function/data/function_call_expression.hpp"
#include "function/data/function_type.hpp"
#include "literal/logic/build_literal_expr.hpp"
#include "operator/logic/build_operator_expr.hpp"
#include "parser/data/node.hpp"
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
  size_t signature_id = 0;
  for (FunctionDefinition &definition : static_cast<FunctionType &>(*callee->type).definitions) {
    auto &signature = *definition.signature;
    arguments.clear();

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
