#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex_shared.hpp"

#include "data/source/number_literal.hpp"

namespace amelia {

struct Type;

enum class ExpressionKind : uint8_t {
  NumberLiteral,
  BooleanLiteral,
  NullLiteral,
  Identifier,
  UnaryOperation,
  BuiltinTypeCast,
  Sequence,
  ValueBinding,
  Empty,
};

struct Expression {
  ExpressionKind kind;
  FlexShared<Type> type;
  NodeId node_id;

  virtual Serialize serialize() const = 0;
  virtual ~Expression();

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

struct NullLiteralExpression : Expression {
  NullLiteralExpression() : Expression(ExpressionKind::NullLiteral) {}
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

struct BuiltinTypeCastExpression : Expression {
  BuiltinTypeCastExpression() : Expression(ExpressionKind::BuiltinTypeCast) {}
  FlexShared<Expression> expr;
  Serialize serialize() const override;
};

struct SequenceExpression : Expression {
  SequenceExpression() : Expression(ExpressionKind::Sequence) {}
  Serialize serialize() const override;
  List<FlexShared<Expression>> exprs;
};

struct ValueBindingExpression : Expression {
  ValueBindingExpression() : Expression(ExpressionKind::ValueBinding) {}
  Serialize serialize() const override;
  String name;
  Option<FlexShared<Expression>> value;
  Option<FlexShared<Expression>> body;
};

struct EmptyExpression : Expression {
  EmptyExpression() : Expression(ExpressionKind::Empty) {}
  Serialize serialize() const override;
};

} // namespace amelia
