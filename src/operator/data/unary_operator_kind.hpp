#pragma once

#include <cstdint>

#include "util/data/serialize.hpp"

namespace amelia {

enum class UnaryOperatorKind : uint8_t {
  Negate,
  Positive,
  Not,
  BitwiseNot,
  Decrement,
  Increment,
};

Serialize serialize_unary_operator_kind(UnaryOperatorKind kind);

} // namespace amelia
