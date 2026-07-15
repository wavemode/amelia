#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct NativeTypeCastExpression : ExpressionWithDynamicId<NativeTypeCastExpression> {
  Flex<Expression> expr;
  Serialize serialize() const override;
};

} // namespace amelia
