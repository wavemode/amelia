#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex_shared.hpp"
#include "data/util/integer.hpp"
#include "data/util/rational.hpp"

namespace amelia {

enum class BuiltinKind : uint8_t {
  Byte,
  UByte,
  Short,
  UShort,
  Int,
  UInt,
  Long,
  ULong,
  USize,
  Float,
  Double,
  Bool,
  Char,
  Str,
  Null,
  Never,
  Unknown,
};

Serialize serialize_builtin_kind(BuiltinKind kind);

enum class TypeKind : uint8_t {
  Alias,
  TypeFn,
  Apply,
  Builtin,
  BitInt,
  Tuple,
  Struct,
  Reference,
  Pointer,
  Array,
  Slice,
  Impl,
  ConstInteger,
  ConstRational,
  ConstBoolean,
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

struct BuiltinType : Type {
  BuiltinType() : Type(TypeKind::Builtin) {}
  BuiltinKind builtin_kind;
};

struct AliasType : Type {
  AliasType() : Type(TypeKind::Alias) {}
  String name;
  String module_name;
  FlexShared<Type> target;
};

struct ConstIntegerType : Type {
  ConstIntegerType() : Type(TypeKind::ConstInteger) {}
  ConstIntegerType(Integer value) : Type(TypeKind::ConstInteger), value(move(value)) {}
  Integer value;
};

struct ConstRationalType : Type {
  ConstRationalType() : Type(TypeKind::ConstRational) {}
  ConstRationalType(Rational value) : Type(TypeKind::ConstRational), value(move(value)) {}
  Rational value;
};

struct ConstBooleanType : Type {
  ConstBooleanType() : Type(TypeKind::ConstBoolean) {}
  ConstBooleanType(bool value) : Type(TypeKind::ConstBoolean), value(value) {}
  bool value;
};

struct FunctionType : Type {
  struct FunctionParameter {
    String name;
    FlexShared<Type> type;
  };

  String name;
  Option<FlexShared<Type>> self_type;
  List<FunctionParameter> parameters;
  FlexShared<Type> return_type;

  FunctionType() : Type(TypeKind::Function) {}

  // TODO: default values
  // TODO: implicit params
  // TODO: variadic
  // TODO: generic
};

} // namespace amelia
