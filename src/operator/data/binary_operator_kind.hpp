#pragma once

#include <cstdint>

#include "util/data/serialize.hpp"

namespace amelia {

enum class BinaryOperatorKind : uint8_t {
  Add,
  Subtract,
  Multiply,
  Divide,
  And,
  BitwiseAnd,
  BitwiseOr,
  BitwiseXor,
  Equals,
  Greater,
  GreaterEquals,
  Less,
  LessEquals,
  LeftShift,
  Modulo,
  NotEquals,
  Or,
  RightShift,
  Assignment,
  BitAndAssignment,
  BitOrAssignment,
  BitXorAssignment,
  DivAssignment,
  LShiftAssignment,
  ModAssignment,
  MulAssignment,
  RShiftAssignment,
  SubAssignment,
  AddAssignment,
};

Serialize serialize_binary_operator_kind(BinaryOperatorKind kind);

} // namespace amelia
