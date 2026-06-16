#include "type.hpp"

namespace amelia {

namespace {

Serialize serialize_primitive(const PrimitiveType &type) {
  return serialize_primitive_kind(type.primitive_kind);
}

} // namespace

Serialize Type::serialize() const {
  switch (kind) {
  case TypeKind::Primitive: {
    const PrimitiveType &primitive_type = static_cast<const PrimitiveType &>(*this);
    return serialize_primitive(primitive_type);
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
    String value;
    const_integer.value.to_string(value);
    Serialize result;
    result.set_object_name("ConstInteger");
    result.add_object_field("value", Serialize::literal(move(value)));
    return result;
  }
  case TypeKind::ConstRational: {
    const ConstRationalType &const_rational = static_cast<const ConstRationalType &>(*this);
    String value;
    const_rational.value.to_fraction_string(value);
    Serialize result;
    result.set_object_name("ConstRational");
    result.add_object_field("value", Serialize::literal(move(value)));
    return result;
  }
  default:
    throw RuntimeError("not implemented");
  }
}

Serialize serialize_primitive_kind(PrimitiveKind kind) {
  String result;
  switch (kind) {
  case PrimitiveKind::Byte:
    result.append("byte");
    break;
  case PrimitiveKind::UByte:
    result.append("ubyte");
    break;
  case PrimitiveKind::Short:
    result.append("short");
    break;
  case PrimitiveKind::UShort:
    result.append("ushort");
    break;
  case PrimitiveKind::Int:
    result.append("int");
    break;
  case PrimitiveKind::UInt:
    result.append("uint");
    break;
  case PrimitiveKind::Long:
    result.append("long");
    break;
  case PrimitiveKind::ULong:
    result.append("ulong");
    break;
  case PrimitiveKind::Float:
    result.append("float");
    break;
  case PrimitiveKind::Double:
    result.append("double");
    break;
  case PrimitiveKind::Bool:
    result.append("bool");
    break;
  case PrimitiveKind::Char:
    result.append("char");
    break;
  case PrimitiveKind::Str:
    result.append("str");
    break;
  case PrimitiveKind::Null:
    result.append("null");
    break;
  case PrimitiveKind::Never:
    result.append("never");
    break;
  case PrimitiveKind::Unknown:
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
  case TypeKind::Primitive:
    result.append("Primitive");
    break;
  case TypeKind::Bitint:
    result.append("Bitint");
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
