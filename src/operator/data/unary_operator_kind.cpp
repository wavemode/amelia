#include "unary_operator_kind.hpp"

#include "util/data/string.hpp"

namespace amelia {

Serialize serialize_unary_operator_kind(UnaryOperatorKind kind) {
  switch (kind) {
  case UnaryOperatorKind::Negate:
    return Serialize::literal("-");
  case UnaryOperatorKind::Positive:
    return Serialize::literal("+");
  case UnaryOperatorKind::Not:
    return Serialize::literal("!");
  case UnaryOperatorKind::BitwiseNot:
    return Serialize::literal("~");
  case UnaryOperatorKind::Decrement:
    return Serialize::literal("--");
  case UnaryOperatorKind::Increment:
    return Serialize::literal("++");
  }
}

} // namespace amelia
