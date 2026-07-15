#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct TypeCastExpression : ExpressionWithDynamicId<TypeCastExpression> {
  Flex<Expression> expr;
  Serialize serialize() const override;
};

} // namespace amelia
