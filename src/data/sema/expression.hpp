#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex_shared.hpp"

#include "data/sema/type.hpp"
#include "data/source/number_literal.hpp"

namespace amelia {

struct Expression {
  FlexShared<Type> type;
  NodeId node_id;

  virtual Serialize serialize() const = 0;
  virtual ~Expression() = default;
};

struct NumberLiteralExpression : Expression {
  NumberLiteral value;

  Serialize serialize() const override;
};

struct IdentifierExpression : Expression {
  String name;

  Serialize serialize() const override;
};

enum class UnaryOperatorKind : uint8_t { Negate };

Serialize serialize_unary_operator_kind(UnaryOperatorKind kind);

struct UnaryOperationExpression : Expression {
  UnaryOperatorKind op_kind;
  FlexShared<Expression> operand;

  Serialize serialize() const override;
};

} // namespace amelia
