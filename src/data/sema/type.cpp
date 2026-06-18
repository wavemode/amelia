#include "type.hpp"

namespace amelia {

namespace {

Serialize serialize_builtin(const BuiltinType &type) {
  return serialize_builtin_kind(type.builtin_kind);
}

} // namespace

Serialize Type::serialize() const {
  switch (kind) {
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
    result.set_object_name("FunctionType");
    result.add_object_field("name", Serialize::quoted(function_type.name));
    if (function_type.self_type.has_value()) {
      result.add_object_field("self_type", function_type.self_type.value()->serialize());
    }
    Serialize parameters;
    for (const auto &param : function_type.parameters) {
      Serialize param_ser;
      param_ser.set_object_name("FunctionParameter");
      param_ser.add_object_field("name", Serialize::quoted(param.name));
      param_ser.add_object_field("type", param.type->serialize());
      parameters.add_list_item(move(param_ser));
    }
    result.add_object_field("parameters", move(parameters));
    result.add_object_field("return_type", function_type.return_type->serialize());
    return result;
  }
  default:
    throw RuntimeError("not implemented");
  }
}

Serialize serialize_builtin_kind(BuiltinKind kind) {
  String result;
  switch (kind) {
  case BuiltinKind::Byte:
    result.append("byte");
    break;
  case BuiltinKind::UByte:
    result.append("ubyte");
    break;
  case BuiltinKind::Short:
    result.append("short");
    break;
  case BuiltinKind::UShort:
    result.append("ushort");
    break;
  case BuiltinKind::Int:
    result.append("int");
    break;
  case BuiltinKind::UInt:
    result.append("uint");
    break;
  case BuiltinKind::Long:
    result.append("long");
    break;
  case BuiltinKind::ULong:
    result.append("ulong");
    break;
  case BuiltinKind::USize:
    result.append("usize");
    break;
  case BuiltinKind::Float:
    result.append("float");
    break;
  case BuiltinKind::Double:
    result.append("double");
    break;
  case BuiltinKind::Bool:
    result.append("bool");
    break;
  case BuiltinKind::Char:
    result.append("char");
    break;
  case BuiltinKind::Str:
    result.append("str");
    break;
  case BuiltinKind::Null:
    result.append("null");
    break;
  case BuiltinKind::Never:
    result.append("never");
    break;
  case BuiltinKind::Unknown:
    result.append("unknown");
    break;
  }
  return Serialize::literal(move(result));
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
  case TypeKind::Variable:
    result.append("Variable");
    break;
  }
  return Serialize::literal(move(result));
}

} // namespace amelia
