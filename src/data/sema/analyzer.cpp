#include <climits>

#include "analyzer.hpp"

#include "data/lexer/lexer.hpp"
#include "data/source/source_location_error.hpp"
#include "data/util/integer.hpp"
#include "data/util/rational.hpp"
#include "data/util/text_utils.hpp"

namespace amelia {

namespace {

BuiltinType BYTE_TYPE(BuiltinKind::Byte);
BuiltinType UBYTE_TYPE(BuiltinKind::UByte);
BuiltinType SHORT_TYPE(BuiltinKind::Short);
BuiltinType USHORT_TYPE(BuiltinKind::UShort);
BuiltinType INT_TYPE(BuiltinKind::Int);
BuiltinType UINT_TYPE(BuiltinKind::UInt);
BuiltinType LONG_TYPE(BuiltinKind::Long);
BuiltinType ULONG_TYPE(BuiltinKind::ULong);
BuiltinType USIZE_TYPE(BuiltinKind::USize);
BuiltinType FLOAT_TYPE(BuiltinKind::Float);
BuiltinType DOUBLE_TYPE(BuiltinKind::Double);
BuiltinType BOOL_TYPE(BuiltinKind::Bool);
BuiltinType CHAR_TYPE(BuiltinKind::Char);
BuiltinType STR_TYPE(BuiltinKind::Str);
BuiltinType NULL_TYPE(BuiltinKind::Null);
BuiltinType NEVER_TYPE(BuiltinKind::Never);
BuiltinType UNKNOWN_TYPE(BuiltinKind::Unknown);

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

bool is_numeric_type_representable(const Type &target_type, const Type &assignment_type) {
  if (target_type.kind == TypeKind::ConstInteger) {
    return assignment_type.kind == TypeKind::ConstInteger &&
           static_cast<const ConstIntegerType &>(target_type).value ==
               static_cast<const ConstIntegerType &>(assignment_type).value;
  }

  if (target_type.kind == TypeKind::ConstRational) {
    return (assignment_type.kind == TypeKind::ConstRational &&
            static_cast<const ConstRationalType &>(target_type).value ==
                static_cast<const ConstRationalType &>(assignment_type).value) ||
           (assignment_type.kind == TypeKind::ConstInteger &&
            static_cast<const ConstRationalType &>(target_type).value ==
                Rational(static_cast<const ConstIntegerType &>(assignment_type).value));
  }

  if (target_type.kind == TypeKind::Builtin) {
    BuiltinKind target_builtin_kind = static_cast<const BuiltinType &>(target_type).builtin_kind;
    if (assignment_type.kind == TypeKind::Builtin) {
      switch (static_cast<const BuiltinType &>(assignment_type).builtin_kind) {
      case BuiltinKind::Byte:
        return can_builtin_type_represent_range(target_type, INT8_MIN, INT8_MAX);
      case BuiltinKind::UByte:
        return can_builtin_type_represent_range(target_type, 0, UINT8_MAX);
      case BuiltinKind::Short:
        return can_builtin_type_represent_range(target_type, INT16_MIN, INT16_MAX);
      case BuiltinKind::UShort:
        return can_builtin_type_represent_range(target_type, 0, UINT16_MAX);
      case BuiltinKind::Int:
        return can_builtin_type_represent_range(target_type, INT32_MIN, INT32_MAX);
      case BuiltinKind::UInt:
        return can_builtin_type_represent_range(target_type, 0, UINT32_MAX);
      case BuiltinKind::Float:
        return target_builtin_kind == BuiltinKind::Float ||
               target_builtin_kind == BuiltinKind::Double;
      case BuiltinKind::Double:
        return target_builtin_kind == BuiltinKind::Double;
      case BuiltinKind::Char:
        return can_builtin_type_represent_range(target_type, 0, UINT32_MAX);
      case BuiltinKind::Long:
        return can_builtin_type_represent_range(target_type, INT64_MIN, INT64_MAX);
      case BuiltinKind::ULong:
        return can_builtin_type_represent_range(target_type, 0, UINT64_MAX);
      case BuiltinKind::USize:
        return target_builtin_kind == BuiltinKind::Float ||
               target_builtin_kind == BuiltinKind::Double;
      default:
        break;
      }
    } else if (assignment_type.kind == TypeKind::ConstInteger) {
      const Integer &value = static_cast<const ConstIntegerType &>(assignment_type).value;
      return can_builtin_type_represent_range(target_type, value, value);
    } else if (assignment_type.kind == TypeKind::ConstRational) {
      const Rational &value = static_cast<const ConstRationalType &>(assignment_type).value;
      return target_builtin_kind == BuiltinKind::Float ||
             target_builtin_kind == BuiltinKind::Double ||
             (value.denominator() == 1 &&
              can_builtin_type_represent_range(target_type, value.numerator(), value.numerator()));
    }
  }

  return false;
}

class SemaWorkerState {
public:
  SemaWorkerState(SemaResult &sema_result, Module &module_obj)
      : m_sema_result(sema_result), m_module_obj(module_obj) {}

  void analyze_module() {
    m_module_obj.analyzed = true;
    collect_top_level_bindings();
    for (const auto [binding_name, binding_id] : m_module_obj.scope->binding_ids) {
      ;
      analyze_binding(*m_module_obj.scope->bindings[binding_id]);
    }
  }

  void analyze_binding(Binding &binding) {
    if (binding.analyzed) {
      return;
    }

    binding.analyzed = true;

    switch (binding.kind) {
    case BindingKind::Variable:
      analyze_let_binding(static_cast<ValueBinding &>(binding));
      if (binding.top_level) {
        disallow_top_level_shadowing(static_cast<ValueBinding &>(binding));
        require_top_level_binding_initializer(static_cast<ValueBinding &>(binding));
      }
      break;
    case BindingKind::Constant:
      analyze_const_binding(static_cast<ValueBinding &>(binding));
      if (binding.top_level) {
        disallow_top_level_shadowing(static_cast<ValueBinding &>(binding));
        require_top_level_binding_initializer(static_cast<ValueBinding &>(binding));
      }
      break;
    default:
      throw RuntimeError("not implemented");
    }
  }

  const Binding &resolve_binding(NodeId node_id) {
    const auto &identifier_node = m_module_obj.ast.get_node(node_id).as_IdentifierNode();
    const Option<BindingId> binding_id = m_module_obj.scope->binding_ids.find(identifier_node.name);
    if (!binding_id.has_value()) {
      String error_message = "Unknown identifier '";
      error_message.append(identifier_node.name);
      error_message.append("'");
      raise_error_at_node_id(node_id, move(error_message));
    }
    Binding &binding = *m_module_obj.scope->bindings[binding_id.value()];
    if (!binding.analyzed) {
      analyze_binding(binding);
    }
    return binding;
  }

  void require_top_level_binding_initializer(const ValueBinding &binding) {
    if (!binding.value.has_value()) {
      String error_message = "Missing initializer for '";
      error_message.append(binding.name);
      error_message.append("' at top level");
      raise_error_at_node_id(binding.decl, move(error_message));
    }
  }

  void disallow_top_level_shadowing(const ValueBinding &binding) {
    if (binding.shadowed_binding.has_value()) {
      String error_message = "Duplicate declaration of '";
      error_message.append(binding.name);
      error_message.append("' at top level");
      raise_error_at_node_id(binding.decl, move(error_message));
    }
  }

  void analyze_let_binding(ValueBinding &binding) {
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

  FlexShared<Expression> expect_expression_of_type(
      FlexShared<Type> &expected_type, NodeId expr_node_id
  ) {
    return unify(expected_type, build_expression(expr_node_id));
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
    case NodeType::NegativeExprNode:
      result = build_expr_negative(expr_node_id);
      break;
    default:
      throw RuntimeError("not implemented");
    }

    return result;
  }

  FlexShared<Expression> build_expr_negative(NodeId expr_node_id) {
    const Node &node = m_module_obj.ast.get_node(expr_node_id);
    const NegativeExprNode &expr_node = node.as_NegativeExprNode();
    auto result = FlexShared<UnaryOperationExpression>::emplace();
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
    const Binding &binding = resolve_binding(node_id);
    switch (binding.kind) {
    case BindingKind::Variable:
    case BindingKind::Constant: {
      auto expr = FlexShared<IdentifierExpression>::emplace();
      expr->name = binding.name;
      expr->type = static_cast<const ValueBinding &>(binding).type.value().weak();
      expr->node_id = node_id;
      return expr;
    }
    default:
      throw RuntimeError("not implemented");
    }
  }

  FlexShared<Type> evaluate_type_expr(NodeId type_expr_node_id) {
    const auto &type_expr_node = m_module_obj.ast.get_node(type_expr_node_id);
    switch (type_expr_node.type()) {
    case NodeType::IdentifierNode: {
      return evaluate_type_name_expr(type_expr_node);
    }
    case NodeType::BuiltinTypeNode: {
      return evaluate_builtin_type_expr(type_expr_node.as_BuiltinTypeNode());
    }
    default:
      throw RuntimeError("not implemented");
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

  FlexShared<Type> evaluate_type_name_expr(const Node &type_name_node) {
    const Option<BindingId> binding_id = m_module_obj.scope->binding_ids.find(
        type_name_node.as_IdentifierNode().name
    );
    if (!binding_id.has_value()) {
      String error_message = "Unknown type name '";
      error_message.append(type_name_node.as_IdentifierNode().name);
      error_message.append("'");
      raise_error_at_node(type_name_node, move(error_message));
    }
    Binding &binding = *m_module_obj.scope->bindings[binding_id.value()];
    if (!binding.analyzed) {
      analyze_binding(binding);
    }
    if (binding.kind != BindingKind::Type && binding.kind != BindingKind::Class &&
        binding.kind != BindingKind::Concept) {
      String error_message = "Expected a type name, but '";
      error_message.append(type_name_node.as_IdentifierNode().name);
      error_message.append("' is not a type");
      raise_error_at_node(type_name_node, move(error_message));
    }
    return static_cast<const TypeBinding &>(binding).type.value();
  }

  FlexShared<Expression> unify(FlexShared<Type> &target_type, const FlexShared<Expression> &expr) {
    const FlexShared<Type> &assignment_type = expr->type;

    if (assignment_type->kind == TypeKind::Builtin) {
      BuiltinKind assignment_builtin_kind = static_cast<const BuiltinType &>(*assignment_type)
                                                .builtin_kind;

      // it is an error to attempt to assign unknown - it indicates a type inference cycle
      if (assignment_builtin_kind == BuiltinKind::Unknown) {
        raise_error_at_node_id(expr->node_id, "Cannot infer type of expression");
      }
    }

    // identical types
    if (target_type == assignment_type) {
      return expr;
    }

    // builtin numeric type conversions
    if (is_numeric_type_representable(target_type, assignment_type)) {
      return expr;
    }

    if (target_type->kind == TypeKind::Builtin) {
      BuiltinKind target_builtin_kind = static_cast<const BuiltinType &>(*target_type).builtin_kind;

      // all types unify with unknown
      if (target_builtin_kind == BuiltinKind::Unknown) {
        target_type = assignment_type;
        return expr;
      }

      // char implicitly converts from Const containing a valid Unicode code point
      if (target_builtin_kind == BuiltinKind::Char) {
        if (assignment_type->kind == TypeKind::ConstInteger) {
          const Integer &value = static_cast<const ConstIntegerType &>(*assignment_type).value;
          if (value >= 0 && value <= UINT32_MAX &&
              CharIterator::is_valid_code_point(value.to_uint32())) {
            return expr;
          }
        } else if (assignment_type->kind == TypeKind::ConstRational) {
          const Rational &value = static_cast<const ConstRationalType &>(*assignment_type).value;
          if (value.denominator() == 1) {
            const Integer &numerator = value.numerator();
            if (numerator >= 0 && numerator <= UINT32_MAX &&
                CharIterator::is_valid_code_point(numerator.to_uint32())) {
              return expr;
            }
          }
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
            return expr;
          default:
            break;
          }
        } else if (assignment_type->kind == TypeKind::ConstInteger) {
          const Integer &value = static_cast<const ConstIntegerType &>(*assignment_type).value;
          if (value >= 0) {
            return expr;
          }
        } else if (assignment_type->kind == TypeKind::ConstRational) {
          const Rational &value = static_cast<const ConstRationalType &>(*assignment_type).value;
          if (value.denominator() == 1 && value.numerator() >= 0) {
            return expr;
          }
        }
      }
    }

    String error_message = "Cannot convert expression of type '";
    assignment_type->serialize().to_string(error_message);
    error_message.append("' to expected type '");
    target_type->serialize().to_string(error_message);
    error_message.append("'");
    raise_error_at_node_id(expr->node_id, move(error_message));
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
      binding->name = move(current_binding_details.name);
      binding->analyzed = false;
      binding->top_level = true;
      m_module_obj.scope->binding_ids.set(binding->name, new_binding_id);
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
    default:
      throw RuntimeError("not implemented");
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
