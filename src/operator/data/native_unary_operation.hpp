#pragma once

#include "expr/data/expression.hpp"
#include "operator/data/unary_operator_kind.hpp"

namespace amelia {

struct NativeUnaryOperationExpression : ExpressionWithDynamicId<NativeUnaryOperationExpression> {
  UnaryOperatorKind op_kind;
  Flex<Expression> operand;
  Serialize serialize() const override;
};

} // namespace amelia
