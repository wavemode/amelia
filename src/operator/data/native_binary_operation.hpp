#pragma once

#include "expr/data/expression.hpp"
#include "operator/data/binary_operator_kind.hpp"

namespace amelia {

struct NativeBinaryOperationExpression : Expression {
  BinaryOperatorKind op_kind;
  Flex<Expression> left;
  Flex<Expression> right;
  Serialize serialize() const override;
};

} // namespace amelia
