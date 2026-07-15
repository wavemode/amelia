#include "binary_operator_kind.hpp"

#include "util/data/string.hpp"

namespace amelia {

Serialize serialize_binary_operator_kind(BinaryOperatorKind kind) {
  switch (kind) {
  case BinaryOperatorKind::Add:
    return Serialize::literal("+");
  case BinaryOperatorKind::Subtract:
    return Serialize::literal("-");
  case BinaryOperatorKind::Multiply:
    return Serialize::literal("*");
  case BinaryOperatorKind::Divide:
    return Serialize::literal("/");
  case BinaryOperatorKind::And:
    return Serialize::literal("&&");
  case BinaryOperatorKind::BitwiseAnd:
    return Serialize::literal("&");
  case BinaryOperatorKind::BitwiseOr:
    return Serialize::literal("|");
  case BinaryOperatorKind::BitwiseXor:
    return Serialize::literal("^");
  case BinaryOperatorKind::Equals:
    return Serialize::literal("==");
  case BinaryOperatorKind::Greater:
    return Serialize::literal(">");
  case BinaryOperatorKind::GreaterEquals:
    return Serialize::literal(">=");
  case BinaryOperatorKind::Less:
    return Serialize::literal("<");
  case BinaryOperatorKind::LessEquals:
    return Serialize::literal("<=");
  case BinaryOperatorKind::LeftShift:
    return Serialize::literal("<<");
  case BinaryOperatorKind::Modulo:
    return Serialize::literal("%");
  case BinaryOperatorKind::NotEquals:
    return Serialize::literal("!=");
  case BinaryOperatorKind::Or:
    return Serialize::literal("||");
  case BinaryOperatorKind::RightShift:
    return Serialize::literal(">>");
  case BinaryOperatorKind::Assignment:
    return Serialize::literal("=");
  case BinaryOperatorKind::BitAndAssignment:
    return Serialize::literal("&=");
  case BinaryOperatorKind::BitOrAssignment:
    return Serialize::literal("|=");
  case BinaryOperatorKind::BitXorAssignment:
    return Serialize::literal("^=");
  case BinaryOperatorKind::DivAssignment:
    return Serialize::literal("/=");
  case BinaryOperatorKind::LShiftAssignment:
    return Serialize::literal("<<=");
  case BinaryOperatorKind::ModAssignment:
    return Serialize::literal("%=");
  case BinaryOperatorKind::MulAssignment:
    return Serialize::literal("*=");
  case BinaryOperatorKind::RShiftAssignment:
    return Serialize::literal(">>=");
  case BinaryOperatorKind::SubAssignment:
    return Serialize::literal("-=");
  case BinaryOperatorKind::AddAssignment:
    return Serialize::literal("+=");
  }
}

} // namespace amelia
