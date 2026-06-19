#include <climits>

#include "analyzer.hpp"

#include "data/lexer/lexer.hpp"
#include "data/source/source_location_error.hpp"
#include "data/util/integer.hpp"
#include "data/util/rational.hpp"
#include "data/util/slice_utils.hpp"
#include "data/util/text_utils.hpp"

namespace amelia {

namespace {

BuiltinType BYTE_TYPE;
BuiltinType UBYTE_TYPE;
BuiltinType SHORT_TYPE;
BuiltinType USHORT_TYPE;
BuiltinType INT_TYPE;
BuiltinType UINT_TYPE;
BuiltinType LONG_TYPE;
BuiltinType ULONG_TYPE;
BuiltinType USIZE_TYPE;
BuiltinType FLOAT_TYPE;
BuiltinType DOUBLE_TYPE;
BuiltinType BOOL_TYPE;
BuiltinType CHAR_TYPE;
BuiltinType STR_TYPE;
BuiltinType NULL_TYPE;
BuiltinType NEVER_TYPE;
BuiltinType UNKNOWN_TYPE;

bool init = []() {
  BYTE_TYPE.builtin_kind = BuiltinKind::Byte;
  UBYTE_TYPE.builtin_kind = BuiltinKind::UByte;
  SHORT_TYPE.builtin_kind = BuiltinKind::Short;
  USHORT_TYPE.builtin_kind = BuiltinKind::UShort;
  INT_TYPE.builtin_kind = BuiltinKind::Int;
  UINT_TYPE.builtin_kind = BuiltinKind::UInt;
  LONG_TYPE.builtin_kind = BuiltinKind::Long;
  ULONG_TYPE.builtin_kind = BuiltinKind::ULong;
  USIZE_TYPE.builtin_kind = BuiltinKind::USize;
  FLOAT_TYPE.builtin_kind = BuiltinKind::Float;
  DOUBLE_TYPE.builtin_kind = BuiltinKind::Double;
  BOOL_TYPE.builtin_kind = BuiltinKind::Bool;
  CHAR_TYPE.builtin_kind = BuiltinKind::Char;
  STR_TYPE.builtin_kind = BuiltinKind::Str;
  NULL_TYPE.builtin_kind = BuiltinKind::Null;
  NEVER_TYPE.builtin_kind = BuiltinKind::Never;
  UNKNOWN_TYPE.builtin_kind = BuiltinKind::Unknown;
  return true;
}();

bool is_unknown_type(const Type &type) {
  return type.kind == TypeKind::Builtin &&
         static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Unknown;
}

bool is_never_type(const Type &type) {
  return type.kind == TypeKind::Builtin &&
         static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Never;
}

bool is_null_type(const Type &type) {
  return type.kind == TypeKind::Builtin &&
         static_cast<const BuiltinType &>(type).builtin_kind == BuiltinKind::Null;
}

bool is_value_binding_node_type(NodeType node_type) {
  return node_type == NodeType::LetDeclNode || node_type == NodeType::ConstDeclNode;
}

bool can_builtin_type_represent_range(const Type &type, const Integer &min, const Integer &max) {
  if (type.kind == TypeKind::Builtin) {
    switch (static_cast<const BuiltinType &>(type).builtin_kind) {
    case BuiltinKind::Null:
      return min == 0 && max == 0;
    case BuiltinKind::Bool:
      return min >= 0 && max <= 1;
    case BuiltinKind::Byte:
      return min >= INT8_MIN && max <= INT8_MAX;
    case BuiltinKind::UByte:
      return min >= 0 && max <= UINT8_MAX;
    case BuiltinKind::Short:
      return min >= INT16_MIN && max <= INT16_MAX;
    case BuiltinKind::UShort:
      return min >= 0 && max <= UINT16_MAX;
    case BuiltinKind::Int:
      return min >= INT32_MIN && max <= INT32_MAX;
    case BuiltinKind::UInt:
      return min >= 0 && max <= UINT32_MAX;
    case BuiltinKind::Long:
      return min >= INT64_MIN && max <= INT64_MAX;
    case BuiltinKind::ULong:
      return min >= 0 && max <= UINT64_MAX;
    case BuiltinKind::Float:
    case BuiltinKind::Double:
      return true;
    default:
      break;
    }
  }
  return false;
}

FlexShared<Expression> builtin_type_cast(Type &target_type, FlexShared<Expression> expr) {
  auto cast_expr = emplace_flex<BuiltinTypeCastExpression>();
  cast_expr->type = FlexShared<Type>::weak(&target_type);
  cast_expr->expr = move(expr);
  return cast_expr;
}

class SemaWorkerState {
public:
  SemaWorkerState(SemaResult &sema_result, Module &module_obj)
      : m_sema_result(sema_result), m_module_obj(module_obj) {}

  void push_binding(FlexShared<Binding> binding) {
    Scope &scope = *m_module_obj.scope;
    Text name = binding->name;
    const Option<BindingId> existing_binding_id = scope.binding_ids.find(name);
    BindingId new_binding_id = scope.bindings.size();
    binding->shadowed_binding = existing_binding_id;
    scope.bindings.push_back(move(binding));
    scope.binding_ids.set(name, new_binding_id);
  }

  void pop_binding() {
    Scope &scope = *m_module_obj.scope;
    if (scope.bindings.size() == 0) {
      throw RuntimeError("Attempted to pop binding from empty scope");
    }
    const Binding &binding = scope.bindings[scope.bindings.size() - 1];
    if (!binding.shadowed_binding.has_value()) {
      scope.binding_ids.remove(binding.name);
    } else {
      scope.binding_ids.set(binding.name, binding.shadowed_binding.value());
    }
    scope.bindings.pop_back();
  }

  FlexShared<Expression> require_unify(
      FlexShared<Type> &target_type, const FlexShared<Expression> &expr
  ) {
    auto unified_expr = unify(target_type, expr);
    if (!unified_expr.has_value()) {
      String error_message = "Cannot convert expression of type '";
      expr->type->serialize().to_string(error_message);
      error_message.append("' to expected type '");
      target_type->serialize().to_string(error_message);
      error_message.append("'");
      raise_error_at_node_id(expr->node_id, move(error_message));
    }
    return unified_expr.value();
  }

  Option<FlexShared<Expression>> unify(
      FlexShared<Type> &target_type, const FlexShared<Expression> &expr
  ) {
    const FlexShared<Type> &assignment_type = expr->type;

    if (assignment_type->kind == TypeKind::Builtin) {
      BuiltinKind assignment_builtin_kind = static_cast<const BuiltinType &>(*assignment_type)
                                                .builtin_kind;

      // it is an error to attempt to assign unknown - it indicates a type inference cycle
      if (assignment_builtin_kind == BuiltinKind::Unknown) {
        raise_error_at_node_id(expr->node_id, "Cannot infer type of expression");
      }

      // never unifies with all types
      if (assignment_builtin_kind == BuiltinKind::Never) {
        return expr;
      }
    }

    // identical types
    if (target_type == assignment_type) {
      return expr;
    }

    if (target_type->kind == TypeKind::Builtin) {
      BuiltinKind target_builtin_kind = static_cast<const BuiltinType &>(*target_type).builtin_kind;

      // all types unify with unknown
      if (target_builtin_kind == BuiltinKind::Unknown) {
        target_type = assignment_type;
        return expr;
      }

      // const and builtin types implicitly convert to types that can represent their range of
      // values
      if (assignment_type->kind == TypeKind::Builtin) {
        switch (static_cast<const BuiltinType &>(*assignment_type).builtin_kind) {
        case BuiltinKind::Byte:
          if (can_builtin_type_represent_range(*target_type, INT8_MIN, INT8_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::UByte:
          if (can_builtin_type_represent_range(*target_type, 0, UINT8_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::Short:
          if (can_builtin_type_represent_range(*target_type, INT16_MIN, INT16_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::UShort:
          if (can_builtin_type_represent_range(*target_type, 0, UINT16_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::Int:
          if (can_builtin_type_represent_range(*target_type, INT32_MIN, INT32_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::UInt:
          if (can_builtin_type_represent_range(*target_type, 0, UINT32_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::Float:
          if (target_builtin_kind == BuiltinKind::Float ||
              target_builtin_kind == BuiltinKind::Double) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::Double:
          if (target_builtin_kind == BuiltinKind::Double) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::Char:
          if (can_builtin_type_represent_range(*target_type, 0, UINT32_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::Long:
          if (can_builtin_type_represent_range(*target_type, INT64_MIN, INT64_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::ULong:
          if (can_builtin_type_represent_range(*target_type, 0, UINT64_MAX)) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        case BuiltinKind::USize:
          if (target_builtin_kind == BuiltinKind::Float ||
              target_builtin_kind == BuiltinKind::Double) {
            return builtin_type_cast(target_type, expr);
          }
          break;
        default:
          break;
        }
      } else if (assignment_type->kind == TypeKind::ConstInteger) {
        const Integer &value = static_cast<const ConstIntegerType &>(*assignment_type).value;
        if (can_builtin_type_represent_range(target_type, value, value)) {
          return builtin_type_cast(target_type, expr);
        }
      } else if (assignment_type->kind == TypeKind::ConstRational) {
        const Rational &value = static_cast<const ConstRationalType &>(*assignment_type).value;
        if (target_builtin_kind == BuiltinKind::Float ||
            target_builtin_kind == BuiltinKind::Double ||
            (value.denominator() == 1 &&
             can_builtin_type_represent_range(target_type, value.numerator(), value.numerator()))) {
          return builtin_type_cast(target_type, expr);
        }
      }

      // char implicitly converts from a const containing a valid Unicode code point
      if (target_builtin_kind == BuiltinKind::Char) {
        if (assignment_type->kind == TypeKind::ConstInteger) {
          const Integer &value = static_cast<const ConstIntegerType &>(*assignment_type).value;
          if (value >= 0 && value <= UINT32_MAX &&
              CharIterator::is_valid_code_point(value.to_uint32())) {
            return builtin_type_cast(target_type, expr);
          }
        } else if (assignment_type->kind == TypeKind::ConstRational) {
          const Rational &value = static_cast<const ConstRationalType &>(*assignment_type).value;
          if (value.denominator() == 1) {
            const Integer &numerator = value.numerator();
            if (numerator >= 0 && numerator <= UINT32_MAX &&
                CharIterator::is_valid_code_point(numerator.to_uint32())) {
              return builtin_type_cast(target_type, expr);
            }
          }
        }
      }

      // bool implicitly converts from Const[true] and Const[false]
      if (target_builtin_kind == BuiltinKind::Bool) {
        if (assignment_type->kind == TypeKind::ConstBoolean) {
          return builtin_type_cast(target_type, expr);
        }
      }

      // usize implicitly converts from any unsigned integral type or positive integral Const
      if (target_builtin_kind == BuiltinKind::USize) {
        if (assignment_type->kind == TypeKind::Builtin) {
          switch (static_cast<const BuiltinType &>(*assignment_type).builtin_kind) {
          case BuiltinKind::UByte:
          case BuiltinKind::UShort:
          case BuiltinKind::UInt:
          case BuiltinKind::ULong:
            return builtin_type_cast(target_type, expr);
          default:
            break;
          }
        } else if (assignment_type->kind == TypeKind::ConstInteger) {
          const Integer &value = static_cast<const ConstIntegerType &>(*assignment_type).value;
          if (value >= 0) {
            return builtin_type_cast(target_type, expr);
          }
        } else if (assignment_type->kind == TypeKind::ConstRational) {
          const Rational &value = static_cast<const ConstRationalType &>(*assignment_type).value;
          if (value.denominator() == 1 && value.numerator() >= 0) {
            return builtin_type_cast(target_type, expr);
          }
        }
      }
    }

    // const integers and const rationals with compatible values implicitly convert to each other
    if (target_type->kind == TypeKind::ConstInteger) {
      if ((assignment_type->kind == TypeKind::ConstInteger &&
           static_cast<const ConstIntegerType &>(*target_type).value ==
               static_cast<const ConstIntegerType &>(*assignment_type).value) ||
          (assignment_type->kind == TypeKind::ConstRational &&
           static_cast<const ConstIntegerType &>(*target_type).value ==
               static_cast<const ConstRationalType &>(*assignment_type).value.numerator() &&
           static_cast<const ConstRationalType &>(*assignment_type).value.denominator() == 1)) {
        return builtin_type_cast(target_type, expr);
      }
    } else if (target_type->kind == TypeKind::ConstRational) {
      if ((assignment_type->kind == TypeKind::ConstRational &&
           static_cast<const ConstRationalType &>(*target_type).value ==
               static_cast<const ConstRationalType &>(*assignment_type).value) ||
          (assignment_type->kind == TypeKind::ConstInteger &&
           static_cast<const ConstRationalType &>(*target_type).value ==
               Rational(static_cast<const ConstIntegerType &>(*assignment_type).value))) {
        return builtin_type_cast(target_type, expr);
      }
    }

    // Const[true] and Const[false] implicitly convert to themselves
    if (target_type->kind == TypeKind::ConstBoolean) {
      if (assignment_type->kind == TypeKind::ConstBoolean) {
        if (static_cast<const ConstBooleanType &>(*target_type).value ==
            static_cast<const ConstBooleanType &>(*assignment_type).value) {
          return builtin_type_cast(target_type, expr);
        }
      }
    }

    return None();
  }

  void analyze_module() {
    m_module_obj.analyzed = true;
    collect_top_level_bindings();
    for (Binding &binding : m_module_obj.scope->bindings) {
      analyze_top_level_binding(binding);
    }
  }

  void analyze_top_level_binding(Binding &binding) {
    switch (binding.kind) {
    case BindingKind::Variable:
      analyze_let_binding(static_cast<ValueBinding &>(binding));
      disallow_shadowing(static_cast<ValueBinding &>(binding));  // TODO: factor out
      require_initializer(static_cast<ValueBinding &>(binding)); // TODO: factor out
      break;
    case BindingKind::Constant:
      analyze_const_binding(static_cast<ValueBinding &>(binding));
      disallow_shadowing(static_cast<ValueBinding &>(binding));  // TODO: factor out
      require_initializer(static_cast<ValueBinding &>(binding)); // TODO: factor out
      break;
    case BindingKind::Function:
      analyze_function_binding(static_cast<ValueBinding &>(binding));
      break;
    default:
      raise_error_at_node_id(binding.decl, "not implemented");
    }
  }

  void analyze_binding(Binding &binding) {
    switch (binding.kind) {
    case BindingKind::Variable:
      analyze_let_binding(static_cast<ValueBinding &>(binding));
      break;
    case BindingKind::Constant:
      analyze_const_binding(static_cast<ValueBinding &>(binding));
      break;
    case BindingKind::Function:
      analyze_function_binding(static_cast<ValueBinding &>(binding));
      break;
    default:
      raise_error_at_node_id(binding.decl, "not implemented");
    }
  }

  FlexShared<Type> resolve_type_binding(NodeId node_id, Text name) {
    const Option<BindingId> binding_id = m_module_obj.scope->binding_ids.find(name);
    if (!binding_id.has_value()) {
      String error_message = "Unknown type name '";
      error_message.append(name);
      error_message.append("'");
      raise_error_at_node_id(node_id, move(error_message));
    }
    Binding &binding = *m_module_obj.scope->bindings[binding_id.value()];
    switch (binding.kind) {
    case BindingKind::Type: {
      if (!static_cast<TypeBinding &>(binding).type.has_value()) {
        analyze_binding(binding);
      }
      return static_cast<TypeBinding &>(binding).type.value();
    }
    default: {
      String error_message = "Identifier '";
      error_message.append(name);
      error_message.append("' is not a type name");
      raise_error_at_node_id(node_id, move(error_message));
    }
    }
  }

  FlexShared<Type> resolve_value_binding(NodeId node_id, Text name) {
    const Option<BindingId> binding_id = m_module_obj.scope->binding_ids.find(name);
    if (!binding_id.has_value()) {
      String error_message = "Unknown identifier '";
      error_message.append(name);
      error_message.append("'");
      raise_error_at_node_id(node_id, move(error_message));
    }
    Binding &binding = *m_module_obj.scope->bindings[binding_id.value()];
    switch (binding.kind) {
    case BindingKind::Constant: {
      if (!static_cast<ValueBinding &>(binding).type.has_value()) {
        analyze_const_binding(static_cast<ValueBinding &>(binding));
      }
      return static_cast<ValueBinding &>(binding).type.value();
    }
    case BindingKind::Variable: {
      if (!static_cast<ValueBinding &>(binding).type.has_value()) {
        analyze_let_binding(static_cast<ValueBinding &>(binding));
      }
      return static_cast<ValueBinding &>(binding).type.value();
    }
    case BindingKind::Function: {
      if (!static_cast<ValueBinding &>(binding).type.has_value()) {
        analyze_function_binding(static_cast<ValueBinding &>(binding));
      }
      return static_cast<ValueBinding &>(binding).type.value();
    }
    default:
      raise_error_at_node_id(node_id, "not implemented");
    }
  }

  void require_initializer(const ValueBinding &binding) {
    if (!binding.value.has_value()) {
      String error_message = "Missing initializer for '";
      error_message.append(binding.name);
      error_message.append("' at top level");
      raise_error_at_node_id(binding.decl, move(error_message));
    }
  }

  void disallow_shadowing(const ValueBinding &binding) {
    if (binding.shadowed_binding.has_value()) {
      String error_message = "Duplicate declaration of '";
      error_message.append(binding.name);
      error_message.append("'");
      raise_error_at_node_id(binding.decl, move(error_message));
    }
  }

  void analyze_let_binding(ValueBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    const auto &decl_node = m_module_obj.ast.get_node(binding.decl).as_LetDeclNode();

    if (decl_node.type.has_value()) {
      binding.type = evaluate_type_expr(decl_node.type.value());
    } else {
      binding.type = FlexShared<Type>::weak(&UNKNOWN_TYPE);
    }
    if (decl_node.expr.has_value()) {
      binding.value = expect_expression_of_type(binding.type.value(), decl_node.expr.value());
    }
  }

  void analyze_const_binding(ValueBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    const auto &decl_node = m_module_obj.ast.get_node(binding.decl).as_ConstDeclNode();

    if (decl_node.type.has_value()) {
      binding.type = evaluate_type_expr(decl_node.type.value());
    } else {
      binding.type = FlexShared<Type>::weak(&UNKNOWN_TYPE);
    }

    if (decl_node.expr.has_value()) {
      binding.value = expect_expression_of_type(binding.type.value(), decl_node.expr.value());
    }
  }

  void analyze_function_binding(ValueBinding &binding) {
    if (binding.type.has_value()) {
      return;
    }

    binding.type = FlexShared<Type>::weak(&UNKNOWN_TYPE);

    auto function_type = FlexShared<FunctionType>::emplace();
    auto *current_binding = &binding;
    while (true) {
      const auto &decl_node = m_module_obj.ast.get_node(current_binding->decl)
                                  .as_FunctionDeclNode();
      function_type->signatures.push_back(analyze_function_signature(decl_node.signature));
      if (current_binding->shadowed_binding.has_value()) {
        auto &shadowed_binding = *m_module_obj.scope
                                      ->bindings[current_binding->shadowed_binding.value()];
        if (shadowed_binding.kind != BindingKind::Function) {
          String error_message = "Function declaration '";
          error_message.append(binding.name);
          error_message.append("' conflicts with previous declaration of the same name");
          raise_error_at_node_id(binding.decl, move(error_message));
        }
        current_binding = static_cast<ValueBinding *>(&shadowed_binding);
      } else {
        break;
      }
    }
    function_type->signatures.reverse();

    binding.type = function_type;

    current_binding = &binding;
    size_t signature_index = function_type->signatures.size() - 1;
    while (true) {
      const auto &decl_node = m_module_obj.ast.get_node(current_binding->decl)
                                  .as_FunctionDeclNode();
      if (!decl_node.body.has_value()) {
        raise_error_at_node_id(
            current_binding->decl, "not implemented (function declaration without body)"
        );
      }
      current_binding->value = analyze_function_body(
          function_type->signatures[signature_index], decl_node.body.value()
      );
      --signature_index;
      if (current_binding->shadowed_binding.has_value()) {
        auto &shadowed_binding = *m_module_obj.scope
                                      ->bindings[current_binding->shadowed_binding.value()];
        current_binding = static_cast<ValueBinding *>(&shadowed_binding);
      } else {
        break;
      }
    }
  }

  FlexShared<Expression> analyze_function_body(
      FunctionType::Signature &signature, NodeId function_body_node_id
  ) {
    Option<FunctionType::Signature *> old_signature = m_current_function_signature;
    m_current_function_signature = &signature;
    for (const auto &param : signature.parameters) {
      auto binding = emplace_flex<ValueBinding>();
      binding->name = param.name;
      binding->type = param.type;
      push_binding(move(binding));
    }

    const auto &function_body_node = m_module_obj.ast.get_node(function_body_node_id)
                                         .as_FunctionBodyNode();

    if (function_body_node.is_default || function_body_node.is_deleted) {
      raise_error_at_node_id(
          function_body_node_id, "not implemented (defaulted or deleted function)"
      );
    }

    FlexShared<Expression> result;
    if (function_body_node.expr.has_value()) {
      auto expr = build_expression(function_body_node.expr.value());
      auto unified_expr = unify(signature.return_type, expr);
      if (!unified_expr.has_value()) {
        String error_message = "Cannot convert expression of type '";
        expr->type->serialize().to_string(error_message);
        error_message.append("' to expected return type '");
        signature.return_type->serialize().to_string(error_message);
        error_message.append("'");
        raise_error_at_node_id(expr->node_id, move(error_message));
      }
      result = unified_expr.value();
    } else {
      result = build_expr_seq(function_body_node_id, function_body_node.stmts.value().data());

      if (is_unknown_type(signature.return_type)) {
        // function had no declared return type, and also contained no return statements
        signature.return_type = FlexShared<Type>::weak(&NULL_TYPE);
      } else if (!is_never_type(result->type) && !is_null_type(signature.return_type)) {
        // function does not return a value on all code paths
        raise_error_at_node_id(
            function_body_node_id, "Non-null function does not return a value on all code paths"
        );
      }
    }

    for (size_t i = 0; i < signature.parameters.size(); ++i) {
      pop_binding();
    }
    m_current_function_signature = old_signature;

    return result;
  }

  FunctionType::Signature analyze_function_signature(NodeId signature_node_id) {
    const auto &signature_node = m_module_obj.ast.get_node(signature_node_id)
                                     .as_FunctionSignatureNode();
    FunctionType::Signature result;
    Set<Text> seen_param_names;
    for (NodeId parameter_node_id : signature_node.parameters) {
      auto param = analyze_function_parameter(parameter_node_id);
      if (seen_param_names.has(param.name)) {
        String error_message = "Duplicate parameter name '";
        error_message.append(param.name);
        error_message.append("' in function signature");
        raise_error_at_node_id(parameter_node_id, move(error_message));
      }
      seen_param_names.add(param.name);
      result.parameters.push_back(move(param));
    }
    if (signature_node.return_type.has_value()) {
      result.return_type = evaluate_type_expr(signature_node.return_type.value());
    } else {
      result.return_type = FlexShared<Type>::weak(&UNKNOWN_TYPE);
    }
    return result;
  }

  FunctionType::Parameter analyze_function_parameter(NodeId parameter_node_id) {
    const auto &parameter_node = m_module_obj.ast.get_node(parameter_node_id)
                                     .as_FunctionParameterNode();
    const auto &parameter_name_node = m_module_obj.ast.get_node(parameter_node.name);
    if (parameter_name_node.type() != NodeType::IdentifierNode) {
      raise_error_at_node_id(parameter_node.name, "not implemented (function param not Ident)");
    }

    FunctionType::Parameter result;
    result.name = parameter_name_node.as_IdentifierNode().name;
    if (!parameter_node.type.has_value()) {
      raise_error_at_node_id(
          parameter_node.type.value(), "not implemented (missing function param type annotation)"
      );
    }
    result.type = evaluate_type_expr(parameter_node.type.value());
    if (parameter_node.default_value.has_value()) {
      result.default_value = expect_expression_of_type(
          result.type, parameter_node.default_value.value()
      );
    }
    return result;
  }

  FlexShared<Expression> expect_expression_of_type(
      FlexShared<Type> &expected_type, NodeId expr_node_id
  ) {
    return require_unify(expected_type, build_expression(expr_node_id));
  }

  FlexShared<Expression> build_expression(NodeId expr_node_id) {
    const auto &expr_node = m_module_obj.ast.get_node(expr_node_id);
    FlexShared<Expression> result;
    switch (expr_node.type()) {
    case NodeType::NumberLiteralNode:
      result = build_expr_number_literal(expr_node_id);
      break;
    case NodeType::IdentifierNode:
      result = build_expr_identifier(expr_node_id);
      break;
    case NodeType::NegateExprNode:
      result = build_expr_negate(expr_node_id);
      break;
    case NodeType::BooleanLiteralNode:
      result = build_expr_boolean_literal(expr_node_id);
      break;
    case NodeType::BuiltinTypeNode:
      result = build_expr_builtin_type(expr_node_id);
      break;
    case NodeType::BlockExprNode:
      result = build_expr_block(expr_node_id);
      break;
    case NodeType::ExprStmtNode:
      result = build_expr_expression_statement(expr_node_id);
      break;
    case NodeType::EmptyStmtNode:
      result = FlexShared<EmptyExpression>::emplace();
      result->node_id = expr_node_id;
      result->type = FlexShared<Type>::weak(&NULL_TYPE);
      break;
    case NodeType::LetDeclNode:
    case NodeType::ConstDeclNode:
      result = build_expr_value_binding(expr_node_id, ConstSlice<NodeId>());
      break;
    case NodeType::ReturnStmtNode:
      result = build_expr_return(expr_node_id);
      break;
    default:
      raise_error_at_node_id(expr_node_id, "not implemented");
    }

    return result;
  }

  FlexShared<Expression> build_expr_return(NodeId expr_node_id) {
    if (!m_current_function_signature.has_value()) {
      raise_error_at_node_id(expr_node_id, "Return statement not within function");
    }

    const auto &return_node = m_module_obj.ast.get_node(expr_node_id).as_ReturnStmtNode();
    auto result = emplace_flex<ReturnExpression>();
    result->node_id = expr_node_id;
    if (return_node.expr.has_value()) {
      result->value = expect_expression_of_type(
          m_current_function_signature.value()->return_type, return_node.expr.value()
      );
    } else {
      auto implied_return_value = FlexShared<NullLiteralExpression>::emplace();
      implied_return_value->node_id = expr_node_id;
      implied_return_value->type = FlexShared<Type>::weak(&NULL_TYPE);
      auto return_value = unify(
          m_current_function_signature.value()->return_type, implied_return_value
      );
      if (!return_value.has_value()) {
        String error_message = "Empty return statement in function returning '";
        m_current_function_signature.value()->return_type->serialize().to_string(error_message);
        error_message.append("'");
        raise_error_at_node_id(expr_node_id, move(error_message));
      }
      result->value = return_value.value();
    }
    result->type = FlexShared<Type>::weak(&NEVER_TYPE);
    return result;
  }

  FlexShared<Expression> build_expr_expression_statement(NodeId expr_node_id) {
    const auto &expr_stmt_node = m_module_obj.ast.get_node(expr_node_id).as_ExprStmtNode();
    return build_expression(expr_stmt_node.expr);
  }

  FlexShared<Expression> build_expr_block(NodeId expr_node_id) {
    const BlockExprNode &block_expr_node = m_module_obj.ast.get_node(expr_node_id)
                                               .as_BlockExprNode();
    if (block_expr_node.stmts.size() == 0) {
      auto result = FlexShared<EmptyExpression>::emplace();
      result->node_id = expr_node_id;
      result->type = FlexShared<Type>::weak(&NULL_TYPE);
      return result;
    } else if (block_expr_node.stmts.size() == 1) {
      return build_expression(block_expr_node.stmts[0]);
    }
    return build_expr_seq(expr_node_id, block_expr_node.stmts.data());
  }

  FlexShared<Expression> build_expr_seq(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    auto result = FlexShared<SequenceExpression>::emplace();
    result->type = FlexShared<Type>::weak(&UNKNOWN_TYPE);
    result->node_id = expr_node_id;
    for (size_t expr_index = 0; expr_index < stmts.size(); ++expr_index) {
      const auto &expr_node = m_module_obj.ast.get_node(stmts[expr_index]);
      if (is_value_binding_node_type(expr_node.type())) {
        auto expr = build_expr_value_binding(
            stmts[expr_index], SliceUtils::tail(stmts, expr_index + 1)
        );
        result->type = expr->type;
        result->exprs.push_back(expr);
        break;
      }

      auto expr = build_expression(stmts[expr_index]);
      result->exprs.push_back(expr);
      if (expr_index == stmts.size() - 1 && !is_never_type(result->type)) {
        result->type = expr->type;
      }
    }
    return result;
  }

  FlexShared<Expression> build_expr_value_binding(NodeId expr_node_id, ConstSlice<NodeId> stmts) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
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
    binding->name = m_module_obj.ast.get_node(target).as_IdentifierNode().name;
    binding->kind = is_const ? BindingKind::Constant : BindingKind::Variable;
    binding->visibility = DeclarationVisibility::Default;
    binding->type = type.has_value() ? evaluate_type_expr(type.value())
                                     : FlexShared<Type>::weak(&UNKNOWN_TYPE);
    binding->value = expr.has_value()
                         ? expect_expression_of_type(binding->type.value(), expr.value())
                         : Option<FlexShared<Expression>>();

    auto result = emplace_flex<ValueBindingExpression>();
    result->name = binding->name;
    result->value = binding->value;
    result->type = FlexShared<Type>::weak(&NULL_TYPE);
    push_binding(move(binding));
    if (stmts.size() == 1) {
      result->body = build_expression(stmts[0]);
      result->type = result->body.value()->type;
    } else if (stmts.size() > 1) {
      result->body = build_expr_seq(stmts[0], stmts);
      result->type = result->body.value()->type;
    }
    pop_binding();
    return result;
  }

  FlexShared<Expression> build_expr_builtin_type(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    const BuiltinTypeNode &expr_node = node.as_BuiltinTypeNode();
    switch (expr_node.kind) {
    case BuiltinKind::Null: {
      auto result = emplace_flex<NullLiteralExpression>();
      result->node_id = expr_node_id;
      result->type = FlexShared<Type>::weak(&NULL_TYPE);
      return result;
    }
    default:
      raise_error_at_node_id(expr_node_id, "not implemented");
    }
  }

  FlexShared<Expression> build_expr_boolean_literal(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    const BooleanLiteralNode &expr_node = node.as_BooleanLiteralNode();
    auto result = emplace_flex<BooleanLiteralExpression>();
    result->node_id = expr_node_id;
    result->value = expr_node.value;
    result->type = make_flex(ConstBooleanType(expr_node.value));
    return result;
  }

  FlexShared<Expression> build_expr_negate(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    const NegateExprNode &expr_node = node.as_NegateExprNode();
    auto result = emplace_flex<UnaryOperationExpression>();
    result->node_id = expr_node_id;
    result->op_kind = UnaryOperatorKind::Negate;
    result->operand = build_expression(expr_node.expr);
    switch (result->operand->type->kind) {
    case TypeKind::Builtin:
      switch (static_cast<const BuiltinType &>(*result->operand->type).builtin_kind) {
      case BuiltinKind::Byte:
      case BuiltinKind::UByte:
      case BuiltinKind::Short:
      case BuiltinKind::UShort:
      case BuiltinKind::Int:
      case BuiltinKind::UInt:
      case BuiltinKind::Long:
      case BuiltinKind::ULong:
      case BuiltinKind::USize:
      case BuiltinKind::Float:
      case BuiltinKind::Double:
      case BuiltinKind::Char:
        result->type = result->operand->type;
        break;
      default:
        goto fail;
      }
      break;

    case TypeKind::ConstInteger: {
      result->type = make_flex(
          ConstIntegerType(-static_cast<const ConstIntegerType &>(*result->operand->type).value)
      );
      break;
    }

    case TypeKind::ConstRational: {
      result->type = make_flex(
          ConstRationalType(-static_cast<const ConstRationalType &>(*result->operand->type).value)
      );
      break;
    }

    default:
      goto fail;
    }
    return result;

  fail:

    String error_message = "Cannot negate expression of type '";
    result->operand->type->serialize().to_string(error_message);
    error_message.append('\'');
    raise_error_at_node_id(expr_node_id, move(error_message));
  }

  FlexShared<Expression> build_expr_identifier(NodeId node_id) {
    const auto &identifier_node = m_module_obj.ast.get_node(node_id).as_IdentifierNode();
    auto expr = FlexShared<IdentifierExpression>::emplace();
    expr->name = identifier_node.name;
    expr->type = resolve_value_binding(node_id, identifier_node.name).weak();
    expr->node_id = node_id;
    return expr;
  }

  FlexShared<Type> evaluate_type_expr(NodeId type_expr_node_id) {
    const auto &type_expr_node = m_module_obj.ast.get_node(type_expr_node_id);
    switch (type_expr_node.type()) {
    case NodeType::IdentifierNode:
      return resolve_type_binding(type_expr_node_id, type_expr_node.as_IdentifierNode().name);
    case NodeType::BuiltinTypeNode:
      return evaluate_builtin_type_expr(type_expr_node.as_BuiltinTypeNode());
    case NodeType::ConstTypeExprNode:
      return evaluate_const_type_expr(type_expr_node.as_ConstTypeExprNode());
    default:
      raise_error_at_node_id(type_expr_node_id, "not implemented");
    }
  }

  FlexShared<Type> evaluate_const_type_expr(const ConstTypeExprNode &const_type_expr_node) {
    auto expr = build_expression(const_type_expr_node.expr);
    switch (expr->type->kind) {
    case TypeKind::ConstInteger:
    case TypeKind::ConstRational:
    case TypeKind::ConstBoolean:
      return expr->type;
    default:
      String error_message = "Expected a constant, but got an expression of type '";
      expr->type->serialize().to_string(error_message);
      error_message.append('\'');
      raise_error_at_node_id(expr->node_id, move(error_message));
    }
  }

  FlexShared<Type> evaluate_builtin_type_expr(const BuiltinTypeNode &builtin_type_node) {
    switch (builtin_type_node.kind) {
    case BuiltinKind::Bool:
      return FlexShared<Type>::weak(&BOOL_TYPE);
    case BuiltinKind::Byte:
      return FlexShared<Type>::weak(&BYTE_TYPE);
    case BuiltinKind::Short:
      return FlexShared<Type>::weak(&SHORT_TYPE);
    case BuiltinKind::Int:
      return FlexShared<Type>::weak(&INT_TYPE);
    case BuiltinKind::Long:
      return FlexShared<Type>::weak(&LONG_TYPE);
    case BuiltinKind::UByte:
      return FlexShared<Type>::weak(&UBYTE_TYPE);
    case BuiltinKind::UShort:
      return FlexShared<Type>::weak(&USHORT_TYPE);
    case BuiltinKind::UInt:
      return FlexShared<Type>::weak(&UINT_TYPE);
    case BuiltinKind::ULong:
      return FlexShared<Type>::weak(&ULONG_TYPE);
    case BuiltinKind::USize:
      return FlexShared<Type>::weak(&USIZE_TYPE);
    case BuiltinKind::Float:
      return FlexShared<Type>::weak(&FLOAT_TYPE);
    case BuiltinKind::Double:
      return FlexShared<Type>::weak(&DOUBLE_TYPE);
    case BuiltinKind::Char:
      return FlexShared<Type>::weak(&CHAR_TYPE);
    case BuiltinKind::Str:
      return FlexShared<Type>::weak(&STR_TYPE);
    case BuiltinKind::Null:
      return FlexShared<Type>::weak(&NULL_TYPE);
    case BuiltinKind::Never:
      return FlexShared<Type>::weak(&NEVER_TYPE);
    default:
      throw RuntimeError("unreachable");
    }
  }

  FlexShared<Expression> build_expr_number_literal(NodeId expr_node_id) {
    auto expr = FlexShared<NumberLiteralExpression>::emplace();
    const auto &expr_node = m_module_obj.ast.get_node(expr_node_id).as_NumberLiteralNode();
    expr->node_id = expr_node_id;
    expr->value = expr_node.value;
    if (expr_node.value.has_decimal_point || expr_node.value.exponent_sign == "-") {
      String num_str;
      num_str.append(expr_node.value.base_prefix);
      num_str.append(expr_node.value.integer_digits);
      num_str.append(".");
      num_str.append(expr_node.value.fractional_digits);
      Rational result(num_str);
      if (expr_node.value.exponent_digits.size() > 0) {
        uint8_t exponent_base = 10;
        if (expr_node.value.exponent_prefix == "p" || expr_node.value.exponent_prefix == "P") {
          exponent_base = 2;
        }
        auto factor = Integer(exponent_base)
                          .pow(Integer(expr_node.value.exponent_digits).to_uint32());
        if (expr_node.value.exponent_sign == "-") {
          result = Rational(result.numerator(), result.denominator() * factor);
        } else {
          result = Rational(result.numerator() * factor, result.denominator());
        }
      }
      expr->type = make_flex(ConstRationalType(move(result)));
    } else {
      String num_str;
      num_str.append(expr_node.value.base_prefix);
      num_str.append(expr_node.value.integer_digits);
      Integer result(num_str);
      if (expr_node.value.exponent_digits.size() > 0) {
        uint8_t exponent_base = 10;
        if (expr_node.value.exponent_prefix == "p" || expr_node.value.exponent_prefix == "P") {
          exponent_base = 2;
        }
        auto factor = Integer(exponent_base)
                          .pow(Integer(expr_node.value.exponent_digits).to_uint32());
        result *= factor;
      }
      expr->type = make_flex(ConstIntegerType(move(result)));
    }
    return expr;
  }

  void collect_top_level_bindings() {
    const ModuleNode &module_node = m_module_obj.ast.get_node(m_module_obj.ast_root)
                                        .as_ModuleNode();
    for (NodeId decl_node_id : module_node.decls) {
      Binding current_binding_details{};
      current_binding_details.visibility = DeclarationVisibility::Default;
      get_binding_details(current_binding_details, decl_node_id);
      const Option<BindingId> existing_binding_id = m_module_obj.scope->binding_ids.find(
          current_binding_details.name
      );
      auto binding = FlexShared<Binding>::emplace();
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
      binding->shadowed_binding = existing_binding_id;
      binding->name = move(current_binding_details.name);
      push_binding(move(binding));
    }
  }

  void get_binding_details(Binding &current_binding_details, NodeId decl_node_id) {
    const Node &decl_node = m_sema_result.modules[0].ast.get_node(decl_node_id);
    switch (decl_node.type()) {
    case NodeType::LetDeclNode: {
      const auto &n = decl_node.as_LetDeclNode();
      current_binding_details.kind = BindingKind::Variable;
      get_binding_details(current_binding_details, n.target);
      break;
    }
    case NodeType::ConstDeclNode: {
      const auto &n = decl_node.as_ConstDeclNode();
      current_binding_details.kind = BindingKind::Constant;
      get_binding_details(current_binding_details, n.target);
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
      get_binding_details(current_binding_details, n.name);
      break;
    }
    default:
      raise_error_at_node_id(decl_node_id, "not implemented");
    }
  }

private:
  [[noreturn]] void raise_error_at_node_id(NodeId node_id, String &&error_message) {
    const Node &node = m_module_obj.ast.get_node(node_id);
    raise_error_at_node(node, move(error_message));
  }

  [[noreturn]] void raise_error_at_node(const Node &node, String &&error_message) {
    const Token &token = m_module_obj.tokens.get_token(node.start_token());
    throw SourceLocationError(token.location, move(error_message));
  }

  SemaResult &m_sema_result;
  Module &m_module_obj;
  Option<FunctionType::Signature *> m_current_function_signature;
};

class SemaState {
public:
  SemaState(SemaResult &sema_result) : m_sema_result(sema_result) {}

  void analyze() {
    set_up_dep_counts();
    Option<ModuleId> module_id;
    while ((module_id = select_module_to_analyze()).has_value()) {
      SemaWorkerState(m_sema_result, m_sema_result.modules[module_id.value()]).analyze_module();
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

  Option<ModuleId> select_module_to_analyze() {
    for (size_t i = 0; i < m_sema_result.modules.size(); ++i) {
      Module &module_obj = m_sema_result.modules[i];
      if (module_obj.analyzed) {
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

void Analyzer::analyze(SemaResult &sema_result) {
  SemaState(sema_result).analyze();
}

} // namespace amelia
