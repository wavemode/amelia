#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex_shared.hpp"
#include "data/util/integer.hpp"
#include "data/util/rational.hpp"

namespace amelia {

enum class PrimitiveKind : uint8_t {
  Byte,
  UByte,
  Short,
  UShort,
  Int,
  UInt,
  Long,
  ULong,
  Float,
  Double,
  Bool,
  Char,
  Str,
  Null,
  Never,
  Unknown,
};

Serialize serialize_primitive_kind(PrimitiveKind kind);

enum class TypeKind : uint8_t {
  Alias,
  TypeFn,
  Apply,
  Primitive,
  Bitint,
  Tuple,
  Struct,
  Reference,
  Pointer,
  Array,
  Slice,
  Impl,
  ConstInteger,
  ConstRational,
  Class,
  Union,
  Concept,
  Function,
  FunctionPointer,
  Variable,
};

Serialize serialize_type_kind(TypeKind kind);

struct Type {
  TypeKind kind;

  Serialize serialize() const;

  virtual ~Type() = default;

protected:
  Type(TypeKind kind) : kind(kind) {}
};

struct PrimitiveType : Type {
  PrimitiveType(PrimitiveKind primitive_kind)
      : Type(TypeKind::Primitive), primitive_kind(primitive_kind) {}

  PrimitiveKind primitive_kind;
};

struct AliasType : Type {
  AliasType(String name, FlexShared<Type> target)
      : Type(TypeKind::Alias), name(move(name)), target(move(target)) {}

  String name;
  FlexShared<Type> target;
};

struct ConstIntegerType : Type {
  ConstIntegerType(Integer value) : Type(TypeKind::ConstInteger), value(move(value)) {}

  Integer value;
};

struct ConstRationalType : Type {
  ConstRationalType(Rational value) : Type(TypeKind::ConstRational), value(move(value)) {}

  Rational value;
};

} // namespace amelia
