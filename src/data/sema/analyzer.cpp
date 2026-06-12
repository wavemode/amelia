#include "analyzer.hpp"

#include "data/lexer/lexer.hpp"
#include "data/source/source_location_error.hpp"

namespace amelia {

namespace {

PrimitiveType UNKOWN_TYPE{{TypeKind::Primitive}, PrimitiveKind::Unknown};

FlexShared<Type> unknown_type() {
  return FlexShared<Type>::weak(&UNKOWN_TYPE);
}

PrimitiveType DOUBLE_TYPE{{TypeKind::Primitive}, PrimitiveKind::Double};
FlexShared<Type> double_type() {
  return FlexShared<Type>::weak(&DOUBLE_TYPE);
}

bool unify(FlexShared<Type> &target, FlexShared<Type> &assignment) {
  if (target->kind == TypeKind::Primitive) {
    PrimitiveType &target_type = static_cast<PrimitiveType &>(*target);
    if (target_type.primitive_kind == PrimitiveKind::Unknown) {
      target = assignment;
      return true;
    }
  }
  return false;
}

bool is_primitive(const Type &type) {
  return type.kind == TypeKind::Primitive;
}

PrimitiveKind primitive_kind(const Type &type) {
  if (!is_primitive(type)) {
    throw RuntimeError("Type is not a primitive");
  }
  return static_cast<const PrimitiveType &>(type).primitive_kind;
}

bool is_floating_point_primitive_type(const Type &type) {
  if (!is_primitive(type)) {
    return false;
  }
  switch (primitive_kind(type)) {
  case PrimitiveKind::Float:
  case PrimitiveKind::Double:
    return true;
  default:
    return false;
  }
}

bool is_integral_primitive_type(const Type &type) {
  if (!is_primitive(type)) {
    return false;
  }
  switch (primitive_kind(type)) {
  case PrimitiveKind::Byte:
  case PrimitiveKind::UByte:
  case PrimitiveKind::Short:
  case PrimitiveKind::UShort:
  case PrimitiveKind::Int:
  case PrimitiveKind::UInt:
  case PrimitiveKind::Long:
  case PrimitiveKind::ULong:
    return true;
  default:
    return false;
  }
}

class SemaWorkerState {
public:
  SemaWorkerState(SemaResult &sema_result, Module &module_obj)
      : m_sema_result(sema_result), m_module_obj(module_obj) {}

  void typecheck_module() {
    collect_bindings();
    typecheck_scope(*m_module_obj.scope);
  }

  void typecheck_scope(Scope &scope) {
    for (const auto [binding_name, binding_id] : scope.binding_ids) {
      Binding &binding = *scope.bindings[binding_id];
      typecheck_binding(scope, binding);
    }
  }

  void typecheck_binding(Scope &scope, Binding &binding) {
    switch (binding.kind) {
    case BindingKind::Variable:
    case BindingKind::Constant:
      typecheck_value_binding(scope, static_cast<ValueBinding &>(binding));
      break;
    default:
      throw RuntimeError("not implemented");
    }
  }

  void typecheck_value_binding(Scope &scope, ValueBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    const auto &decl_node = m_module_obj.ast.get_node(binding.decl).as_LetDeclNode();

    binding.type = unknown_type(); // TODO: handle type annotations
    binding.value = typecheck_expression(binding.type.value(), scope, decl_node.expr.value());
  }

  FlexShared<Expression> typecheck_expression(
      FlexShared<Type> &expected_type, Scope &scope, NodeId expr_node_id
  ) {
    const auto &expr_node = m_module_obj.ast.get_node(expr_node_id);
    FlexShared<Expression> result;
    switch (expr_node.type()) {
    case NodeType::NumberLiteralNode:
      result = typecheck_expr_number_literal(expected_type, scope, expr_node_id);
      break;
    default:
      throw RuntimeError("not implemented");
    }
    if (!unify(expected_type, result->type)) {
      const Location &expr_location = m_module_obj.tokens.get_token(expr_node.start_token())
                                          .location;
      String error_message = "Type error: expected type '";
      expected_type->pretty_print().to_string(error_message);
      error_message.append("' got expression of type '");
      result->type->pretty_print().to_string(error_message);
      error_message.append("'");
      throw SourceLocationError(expr_location, move(error_message));
    }
    return result;
  }

  FlexShared<Expression> typecheck_expr_number_literal(
      FlexShared<Type> &expected_type, Scope &scope, NodeId expr_node_id
  ) {
    auto expr = FlexShared<NumberLiteralExpression>::emplace();
    const auto &expr_node = m_module_obj.ast.get_node(expr_node_id).as_NumberLiteralNode();
    expr->node_id = expr_node_id;
    const Token &literal_token = m_module_obj.tokens.get_token(expr_node.lit);
    expr->value = Lexer::read_number_literal(literal_token.contents);
    const NumberLiteral &value = expr->value;
    const Type &expected_type_ref = *expected_type;
    if (value.has_decimal_point) {
      if (is_floating_point_primitive_type(expected_type_ref)) {
        expr->type = expected_type;
      } else {
        expr->type = double_type();
      }
    } else {
      if (is_integral_primitive_type(expected_type_ref) ||
          is_floating_point_primitive_type(expected_type_ref)) {
        expr->type = expected_type;
      } else {
        expr->type = double_type();
      }
    }
    return expr;
  }

  void collect_bindings() {
    Text current_binding_name;
    Binding current_binding_details{};
    current_binding_details.visibility = DeclarationVisibility::Default;
    const ModuleNode &module_node = m_module_obj.ast.get_node(m_module_obj.ast_root)
                                        .as_ModuleNode();
    for (NodeId decl_node_id : module_node.decls) {
      get_binding_details(current_binding_name, current_binding_details, decl_node_id);
      const Option<BindingId> existing_binding_id = m_module_obj.scope->binding_ids.find(
          current_binding_name
      );
      Option<FlexShared<Binding>> existing_binding;
      if (existing_binding_id.has_value()) {
        existing_binding = m_module_obj.scope->bindings[existing_binding_id.value()];
      }
      BindingId new_binding_id;
      if (existing_binding_id.has_value()) {
        new_binding_id = existing_binding_id.value();
      } else {
        new_binding_id = m_module_obj.scope->bindings.size();
        m_module_obj.scope->bindings.emplace_back();
      }
      FlexShared<Binding> &binding = m_module_obj.scope->bindings[new_binding_id];
      switch (current_binding_details.kind) {
      case BindingKind::Variable:
      case BindingKind::Constant:
      case BindingKind::Function:
        binding = FlexShared<ValueBinding>::emplace();
        break;
      case BindingKind::Type:
      case BindingKind::Class:
      case BindingKind::Concept:
        binding = FlexShared<TypeBinding>::emplace();
        break;
      case BindingKind::Module:
        binding = FlexShared<ModuleBinding>::emplace();
        break;
      }
      binding->decl = decl_node_id;
      binding->kind = current_binding_details.kind;
      binding->visibility = current_binding_details.visibility;
      binding->shadowed_binding = existing_binding;
      m_module_obj.scope->binding_ids.set(current_binding_name, new_binding_id);
    }
  }

  void get_binding_details(
      Text &current_binding_name, Binding &current_binding_details, NodeId decl_node_id
  ) {
    const Node &decl_node = m_sema_result.modules[0].ast.get_node(decl_node_id);
    switch (decl_node.type()) {
    case NodeType::LetDeclNode: {
      const auto &n = decl_node.as_LetDeclNode();
      current_binding_details.kind = BindingKind::Variable;
      get_binding_details(current_binding_name, current_binding_details, n.target);
      break;
    }
    case NodeType::IdentifierNode: {
      const auto &n = decl_node.as_IdentifierNode();
      const Token &token = m_sema_result.modules[0].tokens.get_token(n.token);
      current_binding_name = identifier_text(token);
      break;
    }
    default:
      throw RuntimeError("not implemented");
    }
  }

private:
  SemaResult &m_sema_result;
  Module &m_module_obj;
};

class SemaState {
public:
  SemaState(SemaResult &sema_result) : m_sema_result(sema_result) {}

  void typecheck() {
    set_up_dep_counts();
    Option<ModuleId> module_id;
    while ((module_id = select_module_to_typecheck()).has_value()) {
      SemaWorkerState(m_sema_result, m_sema_result.modules[module_id.value()]).typecheck_module();
    }
  }

private:
  void set_up_dep_counts() {
    while (m_module_dep_counts.size() < m_sema_result.modules.size()) {
      m_module_dep_counts.push_back(0);
    }
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      m_module_dep_counts[i] = m_sema_result.modules[i].imported_ids.size();
    }
  }

  Option<ModuleId> select_module_to_typecheck() {
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      Module &module_obj = m_sema_result.modules[i];
      if (module_obj.scope->bindings.size() != 0) {
        // already analyzed
        continue;
      }
      if (m_module_dep_counts[i] == 0) {
        if (module_obj.group_module_ids.size() != 0) {
          // For cooperation with other threads, we should only ever analyze the "head" of a
          // module group - that is, the module in the group with the lowest ID.
          for (ModuleId group_module_id : module_obj.group_module_ids) {
            if (group_module_id < static_cast<ModuleId>(i)) {
              goto cont;
            }
          }
        }
        return Some(i);
      }
    cont:;
    }
    return None();
  }

  SemaResult &m_sema_result;
  List<uint32_t> m_module_dep_counts;
};

} // namespace

void Analyzer::typecheck(SemaResult &sema_result) {
  SemaState(sema_result).typecheck();
}

} // namespace amelia
