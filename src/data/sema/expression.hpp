#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex_shared.hpp"

#include "data/sema/type.hpp"
#include "data/source/number_literal.hpp"

namespace amelia {

enum class ExpressionKind : uint8_t {
  NumberLiteral,
  BooleanLiteral,
  Identifier,
  UnaryOperation,
};

struct Expression {
  ExpressionKind kind;
  FlexShared<Type> type;
  NodeId node_id;

  virtual Serialize serialize() const = 0;
  virtual ~Expression() = default;

protected:
  Expression(ExpressionKind kind) : kind(kind) {}
};

struct NumberLiteralExpression : Expression {
  NumberLiteralExpression() : Expression(ExpressionKind::NumberLiteral) {}
  NumberLiteral value;
  Serialize serialize() const override;
};

struct BooleanLiteralExpression : Expression {
  BooleanLiteralExpression() : Expression(ExpressionKind::BooleanLiteral) {}
  bool value;
  Serialize serialize() const override;
};

struct IdentifierExpression : Expression {
  IdentifierExpression() : Expression(ExpressionKind::Identifier) {}
  String name;
  Serialize serialize() const override;
};

enum class UnaryOperatorKind : uint8_t { Negate };

Serialize serialize_unary_operator_kind(UnaryOperatorKind kind);

struct UnaryOperationExpression : Expression {
  UnaryOperationExpression() : Expression(ExpressionKind::UnaryOperation) {}
  UnaryOperatorKind op_kind;
  FlexShared<Expression> operand;
  Serialize serialize() const override;
};

} // namespace amelia
