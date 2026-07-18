#include "analysis.hpp"

#include "alias/data/alias_type.hpp"
#include "binding/data/module_binding.hpp"
#include "binding/data/type_binding.hpp"
#include "binding/data/value_binding.hpp"
#include "builtin/data/builtin_type.hpp"
#include "expr/logic/build.hpp"
#include "function/data/function_parameter.hpp"
#include "function/data/function_signature.hpp"
#include "function/data/function_type.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "statement/logic/sequence_exprs.hpp"
#include "type/logic/analysis.hpp"
#include "util/data/set.hpp"
#include "util/data/text.hpp"
#include "sema/data/module_analysis_context.hpp"
#include "sema/data/module.hpp"

namespace amelia {

namespace {

void require_initializer(IModuleAnalysisState &module_state, const ValueBinding &binding) {
  if (!binding.value.has_value()) {
    String error_message = "Missing initializer for '";
    error_message.append(binding.name);
    error_message.append("' at top level");
    module_state.raise_type_error_at_node(binding.decl, move(error_message));
  }
}

void disallow_shadowing(IModuleAnalysisState &module_state, const ValueBinding &binding) {
  if (binding.shadowed_binding_id.has_value()) {
    String error_message = "Duplicate declaration of '";
    error_message.append(binding.name);
    error_message.append("'");
    module_state.raise_type_error_at_node(binding.decl, move(error_message));
  }
}

void disallow_function_from_shadowing_non_function(
    IModuleAnalysisState &module_state, const ValueBinding &binding
) {
  auto *current_binding = &binding;
  while (current_binding->shadowed_binding_id.has_value()) {
    auto &shadowed_binding = *module_state.get_binding_by_id(
        current_binding->shadowed_binding_id.value()
    );
    if (shadowed_binding.kind != BindingKind::Function) {
      String error_message = "Function declaration '";
      error_message.append(binding.name);
      error_message.append("' conflicts with previous declaration of the same name");
      module_state.raise_type_error_at_node(binding.decl, move(error_message));
    }
    current_binding = static_cast<ValueBinding *>(&shadowed_binding);
  }
}

} // namespace

void analyze_top_level_binding(IModuleAnalysisState &module_state, Binding &binding) {
  analyze_binding(module_state, binding);
  switch (binding.kind) {
  case BindingKind::Variable:
    disallow_shadowing(module_state, static_cast<ValueBinding &>(binding));
    require_initializer(module_state, static_cast<ValueBinding &>(binding));
    break;
  case BindingKind::Constant:
    disallow_shadowing(module_state, static_cast<ValueBinding &>(binding));
    require_initializer(module_state, static_cast<ValueBinding &>(binding));
    break;
  case BindingKind::Function:
    disallow_function_from_shadowing_non_function(
        module_state, static_cast<ValueBinding &>(binding)
    );
    break;
  default:
    break;
  }
}

bool is_binding_analyzed(IModuleAnalysisState &module_state, const Binding &binding) {
  switch (binding.kind) {
  case BindingKind::Variable:
  case BindingKind::Constant:
  case BindingKind::Function:
    return static_cast<const ValueBinding &>(binding).type.has_value() &&
           !is_unknown_type(static_cast<const ValueBinding &>(binding).type.value());
    break;
  case BindingKind::Type:
    return static_cast<const TypeBinding &>(binding).type.has_value() &&
           !is_unknown_type(static_cast<const TypeBinding &>(binding).type.value());
    break;
  default:
    module_state.raise_type_error_at_node(
        binding.decl, "not implemented (unknown binding kind in is_binding_analyzed)"
    );
  }
}

void analyze_binding(IModuleAnalysisState &module_state, Binding &binding) {
  auto old_binding_currently_analyzing = module_state.current_context().binding_currently_analyzing;
  module_state.current_context().binding_currently_analyzing = &binding;

  switch (binding.kind) {
  case BindingKind::Variable:
    analyze_let_binding(module_state, static_cast<ValueBinding &>(binding));
    break;
  case BindingKind::Constant:
    analyze_const_binding(module_state, static_cast<ValueBinding &>(binding));
    break;
  case BindingKind::Function:
    analyze_function_binding(module_state, static_cast<ValueBinding &>(binding));
    break;
  case BindingKind::Type:
    analyze_type_binding(module_state, static_cast<TypeBinding &>(binding));
    break;
  default:
    module_state.raise_type_error_at_node(
        binding.decl, "not implemented (unknown binding kind in analyze_binding)"
    );
  }

  module_state.current_context().binding_currently_analyzing = old_binding_currently_analyzing;
}

Flex<TypeBinding> resolve_type_binding(
    IModuleAnalysisState &module_state, NodeId node_id, Text name
) {
  Option<BindingId> binding_id = module_state.get_binding_id_by_name(name);
  if (!binding_id.has_value()) {
    String error_message = "Unknown type name '";
    error_message.append(name);
    error_message.append("'");
    module_state.raise_type_error_at_node(node_id, move(error_message));
  }
  Flex<Binding> binding = module_state.get_binding_by_id(binding_id.value());
  analyze_binding(module_state, binding);
  switch (binding->kind) {
  case BindingKind::Type:
    return binding.downcast<TypeBinding>();
  default: {
    String error_message = "Identifier '";
    error_message.append(name);
    error_message.append("' is not a type name");
    module_state.raise_type_error_at_node(node_id, move(error_message));
  }
  }
}

Flex<ValueBinding> resolve_value_binding(
    IModuleAnalysisState &module_state, NodeId node_id, Text name
) {
  Option<BindingId> binding_id = module_state.get_binding_id_by_name(name);
  if (!binding_id.has_value()) {
    String error_message = "Unknown variable '";
    error_message.append(name);
    error_message.append("'");
    module_state.raise_type_error_at_node(node_id, move(error_message));
  }
  Flex<Binding> binding = module_state.get_binding_by_id(binding_id.value());
  analyze_binding(module_state, binding);
  switch (binding->kind) {
  case BindingKind::Constant:
  case BindingKind::Variable:
  case BindingKind::Function:
    return binding.downcast<ValueBinding>();
  default:
    module_state.raise_type_error_at_node(
        node_id, "not implemented (unknown binding kind in resolve_value_binding)"
    );
  }
}

void collect_top_level_bindings(IModuleAnalysisState &module_state, NodeId module_node_id) {
  const ModuleNode &module_node = module_state.get_node(module_node_id).as_ModuleNode();
  for (NodeId decl_node_id : module_node.decls) {
    Binding current_binding_details{};
    current_binding_details.visibility = DeclarationVisibility::Default;
    get_binding_details(module_state, current_binding_details, decl_node_id);
    const Option<BindingId> existing_binding_id = module_state.get_binding_id_by_name(
        current_binding_details.name
    );
    Flex<Binding> binding;
    switch (current_binding_details.kind) {
    case BindingKind::Variable:
    case BindingKind::Constant:
    case BindingKind::Function:
      binding = emplace_flex<ValueBinding>();
      break;
    case BindingKind::Type:
    case BindingKind::Class:
    case BindingKind::Concept:
      binding = emplace_flex<TypeBinding>();
      break;
    case BindingKind::Module:
      binding = emplace_flex<ModuleBinding>();
      break;
    }
    binding->decl = decl_node_id;
    binding->kind = current_binding_details.kind;
    binding->visibility = current_binding_details.visibility;
    binding->shadowed_binding_id = existing_binding_id;
    binding->name = move(current_binding_details.name);
    module_state.push_binding(move(binding));
  }
}

void get_binding_details(
    IModuleAnalysisState &module_state, Binding &current_binding_details, NodeId decl_node_id
) {
  const Node &decl_node = module_state.get_node(decl_node_id);
  switch (decl_node.type()) {
  case NodeType::LetDeclNode: {
    const auto &n = decl_node.as_LetDeclNode();
    current_binding_details.kind = BindingKind::Variable;
    get_binding_details(module_state, current_binding_details, n.target);
    break;
  }
  case NodeType::ConstDeclNode: {
    const auto &n = decl_node.as_ConstDeclNode();
    current_binding_details.kind = BindingKind::Constant;
    get_binding_details(module_state, current_binding_details, n.target);
    break;
  }
  case NodeType::IdentifierNode: {
    const auto &n = decl_node.as_IdentifierNode();
    current_binding_details.name = n.name;
    break;
  }
  case NodeType::FunctionDeclNode: {
    const FunctionDeclNode &n = decl_node.as_FunctionDeclNode();
    current_binding_details.kind = BindingKind::Function;
    get_binding_details(module_state, current_binding_details, n.name);
    break;
  }
  case NodeType::TypeDeclNode: {
    const TypeDeclNode &n = decl_node.as_TypeDeclNode();
    current_binding_details.kind = BindingKind::Type;
    get_binding_details(module_state, current_binding_details, n.name);
    break;
  }
  default:
    module_state.raise_type_error_at_node(
        decl_node_id, "not implemented (unknown top-level decl node)"
    );
  }
}

void analyze_type_binding(IModuleAnalysisState &module_state, TypeBinding &binding) {
  if (binding.type.has_value()) {
    return;
  }

  binding.type = UNKNOWN_TYPE;

  const auto &node = module_state.get_node(binding.decl);
  if (node.type() != NodeType::TypeDeclNode) {
    module_state.raise_type_error_at_node(
        binding.decl, "not implemented (analyze_type_binding for non-TypeDecl node)"
    );
  }
  const auto &type_decl_node = node.as_TypeDeclNode();

  if (type_decl_node.type_expr.has_value()) {
    auto type = evaluate_type_expr(module_state, type_decl_node.type_expr.value());
    auto result = emplace_flex<AliasType>();
    result->name = binding.name;
    result->module_name = module_state.current_module().name;
    result->target = type->resolve_type();
    binding.type = result;
  } else {
    module_state.raise_type_error_at_node(
        binding.decl, "not implemented (analyze_type_binding for TypeDecl node without type_expr)"
    );
  }
}

void analyze_let_binding(IModuleAnalysisState &module_state, ValueBinding &binding) {
  if (binding.type.has_value()) {
    return;
  }

  const auto &decl_node = module_state.get_node(binding.decl).as_LetDeclNode();

  if (decl_node.type.has_value()) {
    binding.type = evaluate_type_expr(module_state, decl_node.type.value());
    if (decl_node.expr.has_value()) {
      binding.value = expect_expression_of_type(
          module_state, binding.type.value(), decl_node.expr.value()
      );
    }
  } else {
    binding.type = UNKNOWN_TYPE;
    if (decl_node.expr.has_value()) {
      binding.value = build_expression(module_state, decl_node.expr.value());
      binding.type = binding.value.value()->type->remove_comptime_const_from_type();
    }
  }
}

void analyze_const_binding(IModuleAnalysisState &module_state, ValueBinding &binding) {
  if (binding.type.has_value()) {
    return;
  }

  const auto &decl_node = module_state.get_node(binding.decl).as_ConstDeclNode();

  if (decl_node.type.has_value()) {
    binding.type = evaluate_type_expr(module_state, decl_node.type.value());
    if (decl_node.expr.has_value()) {
      binding.value = expect_expression_of_type(
          module_state, binding.type.value(), decl_node.expr.value()
      );
    } else {
      module_state.raise_type_error_at_node(
          binding.decl, "Missing initializer for constant declaration"
      );
    }
  } else {
    binding.type = UNKNOWN_TYPE;
    if (decl_node.expr.has_value()) {
      binding.value = build_expression(module_state, decl_node.expr.value());
      binding.type = binding.value.value()->type;
    } else {
      module_state.raise_type_error_at_node(
          binding.decl, "Missing initializer for constant declaration"
      );
    }
  }
}

FunctionParameter analyze_function_parameter(
    IModuleAnalysisState &module_state, NodeId parameter_node_id
) {
  const auto &parameter_node = module_state.get_node(parameter_node_id).as_FunctionParameterNode();
  const auto &parameter_name_node = module_state.get_node(parameter_node.name);
  if (parameter_name_node.type() != NodeType::IdentifierNode) {
    module_state.raise_type_error_at_node(
        parameter_node.name, "not implemented (function param not Ident)"
    );
  }

  FunctionParameter result;
  result.name = parameter_name_node.as_IdentifierNode().name;
  if (!parameter_node.type.has_value()) {
    module_state.raise_type_error_at_node(
        parameter_node.type.value(), "not implemented (missing function param type annotation)"
    );
  }
  result.type = evaluate_type_expr(module_state, parameter_node.type.value());
  if (parameter_node.default_value.has_value()) {
    result.default_value = expect_expression_of_type(
        module_state, result.type, parameter_node.default_value.value()
    );
  }
  return result;
}

FunctionSignature analyze_function_signature(
    IModuleAnalysisState &module_state, NodeId signature_node_id
) {
  const auto &signature_node = module_state.get_node(signature_node_id).as_FunctionSignatureNode();
  FunctionSignature result;
  Set<Text> seen_param_names;
  for (NodeId parameter_node_id : signature_node.parameters) {
    auto param = analyze_function_parameter(module_state, parameter_node_id);
    if (seen_param_names.has(param.name)) {
      String error_message = "Duplicate parameter name '";
      error_message.append(param.name);
      error_message.append("' in function signature");
      module_state.raise_type_error_at_node(parameter_node_id, move(error_message));
    }
    seen_param_names.add(param.name);
    result.parameters.push_back(move(param));
  }
  if (signature_node.return_type.has_value()) {
    result.return_type = evaluate_type_expr(module_state, signature_node.return_type.value());
  } else {
    result.return_type = UNKNOWN_TYPE;
  }
  return result;
}

void analyze_function_binding(IModuleAnalysisState &module_state, ValueBinding &binding) {
  if (binding.type.has_value()) {
    return;
  }

  binding.type = UNKNOWN_TYPE;

  auto function_type = emplace_flex<FunctionType>();
  function_type->name = binding.name;

  auto *current_binding = &binding;
  while (true) {
    const auto &decl_node = module_state.get_node(current_binding->decl).as_FunctionDeclNode();
    function_type->signatures.push_back(
        make_flex(analyze_function_signature(module_state, decl_node.signature))
    );

    if (!current_binding->shadowed_binding_id.has_value()) {
      break;
    }

    auto &shadowed_binding = *module_state.get_binding_by_id(
        current_binding->shadowed_binding_id.value()
    );
    if (shadowed_binding.kind != BindingKind::Function ||
        static_cast<const ValueBinding &>(shadowed_binding).type.has_value()) {
      // The function declaration we're currently analyzing is shadowing a previous declaration
      // that either isn't a function, or was already analyzed. This generally happens when a
      // local declaration shadows a local or global one.

      // Either way, it should not be part of this overload set.
      break;
    }

    if (shadowed_binding.id.value() != current_binding->id.value() - 1) {
      String error_message = "Overload of function '";
      error_message.append(binding.name);
      error_message.append("' must be declared adjacent to its other overloads");
      module_state.raise_type_error_at_node(binding.decl, move(error_message));
    }

    current_binding = static_cast<ValueBinding *>(&shadowed_binding);
  }
  function_type->signatures.reverse();

  binding.type = function_type;

  current_binding = &binding;
  size_t signature_index = function_type->signatures.size();
  while (true) {
    const auto &decl_node = module_state.get_node(current_binding->decl).as_FunctionDeclNode();
    if (!decl_node.body.has_value()) {
      module_state.raise_type_error_at_node(
          current_binding->decl, "not implemented (function declaration without body)"
      );
    }
    current_binding->value = analyze_function_body(
        module_state, function_type->signatures[signature_index - 1], decl_node.body.value()
    );
    --signature_index;
    if (signature_index > 0) {
      auto &shadowed_binding = *module_state.get_binding_by_id(
          current_binding->shadowed_binding_id.value()
      );
      current_binding = static_cast<ValueBinding *>(&shadowed_binding);
    } else {
      break;
    }
  }
}

Flex<Expression> analyze_function_body(
    IModuleAnalysisState &module_state, FunctionSignature &signature, NodeId function_body_node_id
) {
  Option<FunctionSignature *> old_signature = module_state.current_context().current_function_signature;
  module_state.current_context().current_function_signature = &signature;
  for (const auto &param : signature.parameters) {
    auto binding = emplace_flex<ValueBinding>();
    binding->name = param.name;
    binding->type = param.type->remove_comptime_const_from_type();
    module_state.push_binding(move(binding));
  }

  const auto &function_body_node = module_state.get_node(function_body_node_id)
                                       .as_FunctionBodyNode();

  if (function_body_node.is_default || function_body_node.is_deleted) {
    module_state.raise_type_error_at_node(
        function_body_node_id, "not implemented (defaulted or deleted function)"
    );
  }

  Flex<Expression> result;
  if (function_body_node.expr.has_value()) {
    auto expr = build_expression(module_state, function_body_node.expr.value());
    if (is_unknown_type(signature.return_type)) {
      signature.return_type = expr->type->remove_comptime_const_from_type();
      result = expr;
    } else {
      result = require_coerce(
          module_state,
          signature.return_type,
          expr,
          "Cannot convert expression of type '{1}' to expected return type '{2}'"
      );
    }
  } else {
    result = build_stmt_seq(
        module_state, function_body_node_id, function_body_node.stmts.value().data()
    );

    if (is_unknown_type(signature.return_type)) {
      // function had no declared return type, and also contained no return statements
      signature.return_type = NULL_TYPE;
    } else if (!is_never_type(result->type) && !is_null_type(signature.return_type)) {
      // function body does not return a value on all code paths, and we can't default to null
      module_state.raise_type_error_at_node(
          function_body_node_id, "Non-null function does not return a value on all code paths"
      );
    }
  }

  for (size_t i = 0; i < signature.parameters.size(); ++i) {
    module_state.pop_binding();
  }
  module_state.current_context().current_function_signature = old_signature;

  return result;
}

} // namespace amelia
