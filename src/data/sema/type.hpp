#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"

namespace amelia {

enum class PrimitiveKind : unsigned char {
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

enum class TypeKind : unsigned char {
  Alias,
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
  Const,
  Class,
  Union,
  Concept,
  Function,
  FunctionPointer,
  Variable,
};

struct Type {
  TypeKind kind;
};

struct PrimitiveType : Type {
  PrimitiveKind primitive_kind;
};

void format_type(AbstractString &out, const Type &type);

} // namespace amelia
