#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex.hpp"
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
  Closure,
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

} // namespace amelia
