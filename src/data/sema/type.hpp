#pragma once

#include "prelude.hpp"

#include "data/testing/pretty_print.hpp"
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

PrettyPrint pretty_print_primitive_kind(PrimitiveKind kind);

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

PrettyPrint pretty_print_type_kind(TypeKind kind);

struct Type {
  TypeKind kind;

  PrettyPrint pretty_print() const;
};

struct PrimitiveType : Type {
  PrimitiveKind primitive_kind;
};

} // namespace amelia
