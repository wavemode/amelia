#pragma once

#include <cstdint>

namespace amelia {

class Serialize;

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

} // namespace amelia
