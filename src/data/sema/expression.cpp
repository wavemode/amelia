#include <climits>

#include "expression.hpp"

#include "data/source/char_literal.hpp"

namespace amelia {

Serialize serialize_builtin(const BuiltinType &type) {
  return serialize_builtin_kind(type.builtin_kind);
}

Serialize BuiltinType::serialize() const {
  return serialize_builtin_kind(builtin_kind);
}

Serialize AliasType::serialize() const {
  Serialize result;
  result.set_object_name("Alias");
  result.add_object_field("name", Serialize::literal(name));
  result.add_object_field("target", target->serialize());
  return result;
}

Serialize ConstIntegerType::serialize() const {
  String val("Const[");
  value.to_string(val);
  val.append("]");
  return Serialize::literal(move(val));
}

Serialize ConstRationalType::serialize() const {
  String val("Const[");
  value.to_decimal_string(val, 12);
  val.append("]");
  return Serialize::literal(move(val));
}

Serialize ConstBooleanType::serialize() const {
  String val("Const[");
  val.append(value ? Text("true") : Text("false"));
  val.append("]");
  return Serialize::literal(move(val));
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
  case TypeKind::ConstCharacter:
    result.append("ConstCharacter");
    break;
  case TypeKind::ConstString:
    result.append("ConstString");
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

Serialize BitIntType::serialize() const {
  String repr;
  if (!is_signed) {
    repr.append("u");
  }
  repr.append("bitint[");
  Serialize::of(static_cast<int64_t>(bit_width)).to_string(repr);
  repr.append("]");
  return Serialize::literal(move(repr));
}

Serialize CharLiteralExpression::serialize() const {
  String repr;
  repr.append("CharLiteralExpression[");
  serialize_char_literal(value).to_string(repr);
  repr.append("]");
  return Serialize::literal(move(repr));
}

Serialize ConstCharacterType::serialize() const {
  String repr;
  repr.append("Const[");
  serialize_char_literal(value).to_string(repr);
  repr.append("]");
  return Serialize::literal(move(repr));
}

Serialize ConstStringType::serialize() const {
  String repr;
  repr.append("Const[\"");
  for (uint32_t ch : value) {
    serialize_char_literal(ch, false).to_string(repr);
  }
  repr.append("\"]");
  return Serialize::literal(move(repr));
}

Serialize StringLiteralExpression::serialize() const {
  String repr;
  repr.append("StringLiteralExpression[\"");
  for (uint32_t ch : value) {
    serialize_char_literal(ch, false).to_string(repr);
  }
  repr.append("\"]");
  return Serialize::literal(move(repr));
}

Serialize PointerType::serialize() const {
  String repr;
  repr.append("*");
  if (is_const) {
    repr.append("const ");
  }
  pointee->serialize().to_string(repr);
  return Serialize::literal(move(repr));
}

Serialize SliceType::serialize() const {
  String repr;
  repr.append("[");
  element_type->serialize().to_string(repr);
  repr.append("]");
  return Serialize::literal(move(repr));
}

Serialize ArrayType::serialize() const {
  String repr;
  repr.append("[");
  element_type->serialize().to_string(repr);
  repr.append(", ");
  Integer(size).to_string(repr);
  repr.append("]");
  return Serialize::literal(move(repr));
}

Serialize ArrayLiteralExpression::serialize() const {
  Serialize result;
  result.set_object_name("ArrayLiteralExpression");
  Serialize elements_ser;
  for (const auto &element : elements) {
    elements_ser.add_list_item(element->serialize());
  }
  result.add_object_field("elements", move(elements_ser));
  return result;
}

} // namespace amelia
