#include <climits>

#include "expression.hpp"

namespace amelia {

namespace {

bool can_builtin_type_represent_range(const Type &type, const Integer &min, const Integer &max) {
  if (type.kind == TypeKind::Builtin) {
    switch (static_cast<const BuiltinType &>(type).builtin_kind) {
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
    case BuiltinKind::USize:
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

Flex<Expression> builtin_type_cast(Type &target_type, Flex<Expression> expr) {
  auto cast_expr = emplace_flex<BuiltinTypeCastExpression>();
  cast_expr->type = Flex<Type>::weak(&target_type);
  cast_expr->expr = move(expr);
  return cast_expr;
}

} // namespace

Type &Type::resolve() {
  return *this;
}

Option<Flex<Expression>> Type::unify(Flex<Type> &target, Flex<Expression> &expr) {
  if (unify_exact(target->resolve())) {
    return expr;
  }
  return None();
}

bool Type::is_trivial() {
  // TODO
  return true;
}

Serialize serialize_builtin(const BuiltinType &type) {
  return serialize_builtin_kind(type.builtin_kind);
}

Serialize InferredType::serialize() const {
  return target->serialize();
}

bool InferredType::unify_exact(Type &other) {
  return this == &other || target->unify_exact(other);
}

Type &InferredType::resolve() {
  return *target;
}

Serialize BuiltinType::serialize() const {
  return serialize_builtin_kind(builtin_kind);
}

bool BuiltinType::unify_exact(Type &other) {
  return this == &other || builtin_kind == BuiltinKind::Never ||
         (other.kind == TypeKind::Builtin &&
          builtin_kind == static_cast<const BuiltinType &>(other).builtin_kind);
}

Option<Flex<Expression>> BuiltinType::unify(Flex<Type> &target, Flex<Expression> &expr) {
  auto result = Type::unify(target, expr);
  if (!result.has_value()) {
    switch (builtin_kind) {
    case BuiltinKind::Byte:
      if (can_builtin_type_represent_range(target, INT8_MIN, INT8_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::UByte:
      if (can_builtin_type_represent_range(target, 0, UINT8_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::Short:
      if (can_builtin_type_represent_range(target, INT16_MIN, INT16_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::UShort:
      if (can_builtin_type_represent_range(target, 0, UINT16_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::Int:
      if (can_builtin_type_represent_range(target, INT32_MIN, INT32_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::UInt:
      if (can_builtin_type_represent_range(target, 0, UINT32_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::Long:
      if (can_builtin_type_represent_range(target, INT64_MIN, INT64_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::ULong:
      if (can_builtin_type_represent_range(target, 0, UINT64_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::USize:
      if (can_builtin_type_represent_range(target, 0, UINT64_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    case BuiltinKind::Float:
      if (target->kind == TypeKind::Builtin) {
        auto target_builtin_kind = static_cast<const BuiltinType &>(*target).builtin_kind;
        if (target_builtin_kind == BuiltinKind::Float ||
            target_builtin_kind == BuiltinKind::Double) {
          result = builtin_type_cast(target, expr);
        }
      }
      break;
    case BuiltinKind::Double:
      if (target->kind == TypeKind::Builtin) {
        auto target_builtin_kind = static_cast<const BuiltinType &>(*target).builtin_kind;
        if (target_builtin_kind == BuiltinKind::Double) {
          result = builtin_type_cast(target, expr);
        }
      }
      break;
    case BuiltinKind::Char:
      if (can_builtin_type_represent_range(target, 0, UINT32_MAX)) {
        result = builtin_type_cast(target, expr);
      }
      break;
    default:
      break;
    }
  }
  return result;
}

Serialize AliasType::serialize() const {
  Serialize result;
  result.set_object_name("Alias");
  result.add_object_field("name", Serialize::literal(name));
  result.add_object_field("target", target->serialize());
  return result;
}

bool AliasType::unify_exact(Type &other) {
  return this == &other || target->unify_exact(other);
}

Type &AliasType::resolve() {
  return *target;
}

Serialize ConstIntegerType::serialize() const {
  String val("Const[");
  value.to_string(val);
  val.append("]");
  return Serialize::literal(move(val));
}

bool ConstIntegerType::unify_exact(Type &other) {
  return this == &other || (other.kind == TypeKind::ConstInteger &&
                            value == static_cast<const ConstIntegerType &>(other).value);
}

Option<Flex<Expression>> ConstIntegerType::unify(Flex<Type> &target, Flex<Expression> &expr) {
  auto result = Type::unify(target, expr);
  if (!result.has_value()) {
    if (can_builtin_type_represent_range(target, value, value)) {
      result = builtin_type_cast(target, expr);
    }
  }
  return result;
}

Serialize ConstRationalType::serialize() const {
  String val("Const[");
  value.to_fraction_string(val);
  val.append("]");
  return Serialize::literal(move(val));
}

bool ConstRationalType::unify_exact(Type &other) {
  return this == &other || (other.kind == TypeKind::ConstRational &&
                            value == static_cast<const ConstRationalType &>(other).value);
}

Option<Flex<Expression>> ConstRationalType::unify(Flex<Type> &target, Flex<Expression> &expr) {
  auto result = Type::unify(target, expr);
  if (!result.has_value()) {
    if (target->kind == TypeKind::Builtin) {
      auto target_builtin_kind = static_cast<const BuiltinType &>(*target).builtin_kind;
      if (target_builtin_kind == BuiltinKind::Float || target_builtin_kind == BuiltinKind::Double) {
        result = builtin_type_cast(target, expr);
      }
    }
  }
  return result;
}

Serialize ConstBooleanType::serialize() const {
  String val("Const[");
  val.append(value ? Text("true") : Text("false"));
  val.append("]");
  return Serialize::literal(move(val));
}

bool ConstBooleanType::unify_exact(Type &other) {
  return this == &other || (other.kind == TypeKind::ConstBoolean &&
                            value == static_cast<const ConstBooleanType &>(other).value);
}

Option<Flex<Expression>> ConstBooleanType::unify(Flex<Type> &target, Flex<Expression> &expr) {
  auto result = Type::unify(target, expr);
  if (!result.has_value()) {
    if (target->kind == TypeKind::Builtin) {
      auto target_builtin_kind = static_cast<const BuiltinType &>(*target).builtin_kind;
      if (target_builtin_kind == BuiltinKind::Bool) {
        result = builtin_type_cast(target, expr);
      }
    }
  }
  return result;
}

Serialize FunctionType::serialize() const {
  Serialize result;
  Serialize signatures_list;
  for (const auto &signature : signatures) {
    signatures_list.add_list_item(signature.serialize());
  }
  result.set_object_name("FunctionType");
  result.add_object_field("signatures", move(signatures_list));
  return result;
}

bool FunctionType::unify_exact(Type &) {
  // exact unification for Function types is not well-defined
  return false;
}

Serialize ReferenceType::serialize() const {
  String val("&");
  if (is_const) {
    val.append("const ");
  } else if (is_move) {
    val.append("move ");
  }
  referent->serialize().to_string(val);
  return Serialize::literal(move(val));
}

bool ReferenceType::unify_exact(Type &other) {
  if (this == &other) {
    return true;
  }
  if (other.kind != TypeKind::Reference) {
    return false;
  }
  ReferenceType &other_ref_type = static_cast<ReferenceType &>(other);
  return is_const == other_ref_type.is_const && is_move == other_ref_type.is_move &&
         referent->unify_exact(*other_ref_type.referent);
}

Serialize TupleType::serialize() const {
  String val("(");
  for (size_t i = 0; i < element_types.size(); ++i) {
    element_types[i]->serialize().to_string(val);
    if (i < element_types.size() - 1) {
      val.append(", ");
    }
  }
  val.append(")");
  return Serialize::literal(move(val));
}

bool TupleType::unify_exact(Type &other) {
  if (this == &other) {
    return true;
  }
  if (other.kind != TypeKind::Tuple) {
    return false;
  }
  TupleType &other_tuple_type = static_cast<TupleType &>(other);
  if (element_types.size() != other_tuple_type.element_types.size()) {
    return false;
  }
  for (size_t i = 0; i < element_types.size(); ++i) {
    if (!element_types[i]->resolve().unify_exact(other_tuple_type.element_types[i]->resolve())) {
      return false;
    }
  }
  return true;
}

Serialize FunctionType::Signature::serialize() const {
  Serialize result;
  result.set_object_name("Signature");
  if (parameters.size() > 0) {
    Serialize parameters_list;
    for (const auto &parameter : parameters) {
      Serialize parameter_ser;
      parameter_ser.set_object_name("Parameter");
      parameter_ser.add_object_field("name", Serialize::quoted(parameter.name));
      parameter_ser.add_object_field("type", parameter.type->serialize());
      if (parameter.default_value.has_value()) {
        parameter_ser.add_object_field(
            "default_value", parameter.default_value.value()->serialize()
        );
      }
      parameters_list.add_list_item(move(parameter_ser));
    }
    result.add_object_field("parameters", move(parameters_list));
  }
  result.add_object_field("return_type", return_type->serialize());
  return result;
}

Serialize serialize_type_kind(TypeKind kind) {
  String result;
  switch (kind) {
  case TypeKind::Inferred:
    result.append("Inferred");
    break;
  case TypeKind::Alias:
    result.append("Alias");
    break;
  case TypeKind::TypeFn:
    result.append("TypeFn");
    break;
  case TypeKind::Apply:
    result.append("Apply");
    break;
  case TypeKind::Builtin:
    result.append("Builtin");
    break;
  case TypeKind::BitInt:
    result.append("BitInt");
    break;
  case TypeKind::Tuple:
    result.append("Tuple");
    break;
  case TypeKind::Struct:
    result.append("Struct");
    break;
  case TypeKind::Reference:
    result.append("Reference");
    break;
  case TypeKind::Pointer:
    result.append("Pointer");
    break;
  case TypeKind::Array:
    result.append("Array");
    break;
  case TypeKind::Slice:
    result.append("Slice");
    break;
  case TypeKind::Impl:
    result.append("Impl");
    break;
  case TypeKind::ConstInteger:
    result.append("ConstInteger");
    break;
  case TypeKind::ConstRational:
    result.append("ConstRational");
    break;
  case TypeKind::ConstBoolean:
    result.append("ConstBoolean");
    break;
  case TypeKind::Class:
    result.append("Class");
    break;
  case TypeKind::Union:
    result.append("Union");
    break;
  case TypeKind::Concept:
    result.append("Concept");
    break;
  case TypeKind::Function:
    result.append("Function");
    break;
  case TypeKind::FunctionPointer:
    result.append("FunctionPointer");
    break;
  case TypeKind::Closure:
    result.append("Closure");
    break;
  case TypeKind::Variable:
    result.append("Variable");
    break;
  }
  return Serialize::literal(move(result));
}

Expression::~Expression() = default;

Serialize NumberLiteralExpression::serialize() const {
  Serialize result;
  result.set_object_name("NumberLiteralExpression");
  result.add_object_field("lit", serialize_number_literal(value));
  return result;
}

Serialize IdentifierExpression::serialize() const {
  Serialize result;
  result.set_object_name("IdentifierExpression");
  result.add_object_field("name", Serialize::quoted(name));
  return result;
}

Serialize serialize_unary_operator_kind(UnaryOperatorKind kind) {
  switch (kind) {
  case UnaryOperatorKind::Negate:
    return Serialize::literal("Negate");
  default:
    throw RuntimeError("not implemented");
  }
}

Serialize UnaryOperationExpression::serialize() const {
  Serialize result;
  result.set_object_name("UnaryOperationExpression");
  result.add_object_field("op_kind", serialize_unary_operator_kind(op_kind));
  result.add_object_field("operand", operand->serialize());
  return result;
}

Serialize BooleanLiteralExpression::serialize() const {
  Serialize result;
  result.set_object_name("BooleanLiteralExpression");
  result.add_object_field("value", Serialize::of(value));
  return result;
}

Serialize NullLiteralExpression::serialize() const {
  Serialize result;
  result.set_object_name("NullLiteralExpression");
  return result;
}

Serialize BuiltinTypeCastExpression::serialize() const {
  Serialize result;
  result.set_object_name("BuiltinTypeCastExpression");
  result.add_object_field("expr", expr->serialize());
  return result;
}

Serialize SequenceExpression::serialize() const {
  Serialize result;
  result.set_object_name("SequenceExpression");
  Serialize exprs_ser;
  if (exprs.size() > 0) {
    for (const auto &expr : exprs) {
      exprs_ser.add_list_item(expr->serialize());
    }
    result.add_object_field("exprs", move(exprs_ser));
  }
  return result;
}

Serialize ValueBindingExpression::serialize() const {
  Serialize result;
  result.set_object_name("ValueBindingExpression");
  result.add_object_field("name", Serialize::quoted(name));
  if (binding_type.has_value()) {
    result.add_object_field("binding_type", binding_type.value()->serialize());
  }
  if (binding_value.has_value()) {
    result.add_object_field("binding_value", binding_value.value()->serialize());
  }
  if (body.has_value()) {
    result.add_object_field("body", body.value()->serialize());
  }
  return result;
}

Serialize EmptyExpression::serialize() const {
  return Serialize::literal("EmptyExpression()");
}

Serialize ReturnExpression::serialize() const {
  Serialize result;
  result.set_object_name("ReturnExpression");
  if (value.has_value()) {
    result.add_object_field("value", value.value()->serialize());
  }
  return result;
}

Serialize FunctionCallExpression::serialize() const {
  Serialize result;
  result.set_object_name("FunctionCallExpression");
  result.add_object_field("callee", callee->serialize());
  result.add_object_field("signature", signature->serialize());
  Serialize args_ser;
  for (const auto &arg : arguments) {
    if (arg.has_value()) {
      args_ser.add_list_item(arg.value()->serialize());
    } else {
      args_ser.add_list_item(Serialize::literal("(default)"));
    }
  }
  result.add_object_field("arguments", move(args_ser));
  return result;
}

Serialize ConstIntegerExpression::serialize() const {
  Serialize result;
  result.set_object_name("ConstIntegerExpression");
  String val;
  value.to_string(val);
  result.add_object_field("value", Serialize::literal(move(val)));
  return result;
}

Serialize ConstRationalExpression::serialize() const {
  Serialize result;
  result.set_object_name("ConstRationalExpression");
  String val;
  value.to_fraction_string(val);
  result.add_object_field("value", Serialize::literal(move(val)));
  return result;
}

Serialize ConstBooleanExpression::serialize() const {
  Serialize result;
  result.set_object_name("ConstBooleanExpression");
  result.add_object_field("value", Serialize::of(value));
  return result;
}

Serialize AddressOfExpression::serialize() const {
  Serialize result;
  result.set_object_name("AddressOfExpression");
  result.add_object_field("operand", operand->serialize());
  return result;
}

Serialize TupleExpression::serialize() const {
  Serialize result;
  result.set_object_name("TupleExpression");
  Serialize elements_ser;
  for (const auto &element : elements) {
    elements_ser.add_list_item(element->serialize());
  }
  result.add_object_field("elements", move(elements_ser));
  return result;
}

} // namespace amelia
