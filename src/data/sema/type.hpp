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
  BuiltinType(BuiltinKind builtin_kind) : Type(TypeKind::Builtin), builtin_kind(builtin_kind) {}

  BuiltinKind builtin_kind;
};

struct AliasType : Type {
  AliasType(String name_param, String module_name_param, FlexShared<Type> target_param)
      : Type(TypeKind::Alias), name(move(name_param)), module_name(move(module_name_param)),
        target(move(target_param)) {}

  String name;
  String module_name;
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
