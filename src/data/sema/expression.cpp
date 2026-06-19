#include "expression.hpp"

#include "data/sema/type.hpp"

namespace amelia {

Expression::~Expression() = default;

Serialize NumberLiteralExpression::serialize() const {
  Serialize result;
  result.set_object_name("NumberLiteralExpression");
  result.add_object_field("lit", serialize_number_literal(value));
  return result;
}

Serialize IdentifierExpression::serialize() const {
  Serialize result;
  result.set_object_name("IdentifierExpression");
  result.add_object_field("name", Serialize::quoted(name));
  return result;
}

Serialize serialize_unary_operator_kind(UnaryOperatorKind kind) {
  switch (kind) {
  case UnaryOperatorKind::Negate:
    return Serialize::literal("Negate");
  default:
    throw RuntimeError("not implemented");
  }
}

Serialize UnaryOperationExpression::serialize() const {
  Serialize result;
  result.set_object_name("UnaryOperationExpression");
  result.add_object_field("op_kind", serialize_unary_operator_kind(op_kind));
  result.add_object_field("operand", operand->serialize());
  return result;
}

Serialize BooleanLiteralExpression::serialize() const {
  Serialize result;
  result.set_object_name("BooleanLiteralExpression");
  result.add_object_field("value", Serialize::of(value));
  return result;
}

Serialize NullLiteralExpression::serialize() const {
  Serialize result;
  result.set_object_name("NullLiteralExpression");
  return result;
}

Serialize BuiltinTypeCastExpression::serialize() const {
  Serialize result;
  result.set_object_name("BuiltinTypeCastExpression");
  result.add_object_field("expr", expr->serialize());
  return result;
}

Serialize SequenceExpression::serialize() const {
  Serialize result;
  result.set_object_name("SequenceExpression");
  Serialize exprs_ser;
  for (const auto &expr : exprs) {
    exprs_ser.add_list_item(expr->serialize());
  }
  result.add_object_field("exprs", move(exprs_ser));
  return result;
}

Serialize ValueBindingExpression::serialize() const {
  Serialize result;
  result.set_object_name("ValueBindingExpression");
  result.add_object_field("name", Serialize::quoted(name));
  result.add_object_field("type", type->serialize());
  if (value.has_value()) {
    result.add_object_field("value", value.value()->serialize());
  }
  if (body.has_value()) {
    result.add_object_field("body", body.value()->serialize());
  }
  return result;
}

Serialize EmptyExpression::serialize() const {
  return Serialize::literal("EmptyExpression()");
}

Serialize ReturnExpression::serialize() const {
  Serialize result;
  result.set_object_name("ReturnExpression");
  if (value.has_value()) {
    result.add_object_field("value", value.value()->serialize());
  }
  return result;
}

} // namespace amelia
