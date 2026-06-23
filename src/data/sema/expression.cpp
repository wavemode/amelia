#include "expression.hpp"

namespace amelia {

Serialize serialize_builtin(const BuiltinType &type) {
  return serialize_builtin_kind(type.builtin_kind);
}

Serialize Type::serialize() const {
  switch (kind) {
  case TypeKind::Inferred: {
    return static_cast<const InferredType &>(*this).target->serialize();
  }
  case TypeKind::Builtin: {
    const BuiltinType &builtin_type = static_cast<const BuiltinType &>(*this);
    return serialize_builtin(builtin_type);
  }
  case TypeKind::Alias: {
    const AliasType &alias = static_cast<const AliasType &>(*this);
    Serialize result;
    result.set_object_name("Alias");
    result.add_object_field("name", Serialize::literal(alias.name));
    result.add_object_field("target", alias.target->serialize());
    return result;
  }
  case TypeKind::ConstInteger: {
    const ConstIntegerType &const_integer = static_cast<const ConstIntegerType &>(*this);
    String value("Const[");
    const_integer.value.to_string(value);
    value.append("]");
    return Serialize::literal(move(value));
  }
  case TypeKind::ConstRational: {
    const ConstRationalType &const_rational = static_cast<const ConstRationalType &>(*this);
    String value("Const[");
    const_rational.value.to_fraction_string(value);
    value.append("]");
    return Serialize::literal(move(value));
  }
  case TypeKind::ConstBoolean: {
    const ConstBooleanType &const_boolean = static_cast<const ConstBooleanType &>(*this);
    String value("Const[");
    value.append(const_boolean.value ? Text("true") : Text("false"));
    value.append("]");
    return Serialize::literal(move(value));
  }
  case TypeKind::Function: {
    const FunctionType &function_type = static_cast<const FunctionType &>(*this);
    Serialize result;
    Serialize signatures_list;
    for (const auto &signature : function_type.signatures) {
      signatures_list.add_list_item(signature.serialize());
    }
    result.set_object_name("FunctionType");
    result.add_object_field("signatures", move(signatures_list));
    return result;
  }
  case TypeKind::Reference: {
    const ReferenceType &reference_type = static_cast<const ReferenceType &>(*this);
    String value("&");
    if (reference_type.is_const) {
      value.append("const ");
    } else if (reference_type.is_move) {
      value.append("move ");
    }
    reference_type.referent->serialize().to_string(value);
    return Serialize::literal(move(value));
  }
  case TypeKind::Tuple: {
    const TupleType &tuple_type = static_cast<const TupleType &>(*this);
    String value("(");
    for (size_t i = 0; i < tuple_type.element_types.size(); ++i) {
      tuple_type.element_types[i]->serialize().to_string(value);
      if (i < tuple_type.element_types.size() - 1) {
        value.append(", ");
      }
    }
    value.append(")");
    return Serialize::literal(move(value));
  }
  default:
    throw RuntimeError("not implemented");
  }
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
