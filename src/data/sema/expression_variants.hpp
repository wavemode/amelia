#pragma once

#include "prelude.hpp"

#include "data/sema/expression.hpp"
#include "data/source/number_literal.hpp"
#include "data/testing/serialize.hpp"
#include "data/util/flex.hpp"

#include "data/sema/type.hpp"

#include "data/sema/type_variants.hpp"

namespace amelia {

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
  Flex<Expression> operand;
  Serialize serialize() const override;
};

struct BuiltinTypeCastExpression : Expression {
  BuiltinTypeCastExpression() : Expression(ExpressionKind::BuiltinTypeCast) {}
  Flex<Expression> expr;
  Serialize serialize() const override;
};

struct SequenceExpression : Expression {
  SequenceExpression() : Expression(ExpressionKind::Sequence) {}
  Serialize serialize() const override;
  List<Flex<Expression>> exprs;
};

struct ValueBindingExpression : Expression {
  ValueBindingExpression() : Expression(ExpressionKind::ValueBinding) {}
  Serialize serialize() const override;
  String name;
  Option<Flex<Type>> binding_type;
  Option<Flex<Expression>> binding_value;
  Option<Flex<Expression>> body;
};

struct EmptyExpression : Expression {
  EmptyExpression() : Expression(ExpressionKind::Empty) {}
  Serialize serialize() const override;
};

struct ReturnExpression : Expression {
  ReturnExpression() : Expression(ExpressionKind::Empty) {}
  Option<Flex<Expression>> value;
  Serialize serialize() const override;
};

struct FunctionCallExpression : Expression {
  FunctionCallExpression() : Expression(ExpressionKind::FunctionCall) {}
  Flex<Expression> callee;
  FunctionType::Signature *signature;
  List<Option<Flex<Expression>>> arguments;
  Serialize serialize() const override;
};

} // namespace amelia
