#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct NativeTypeCastExpression : Expression {
  Flex<Expression> expr;
  Serialize serialize() const override;
};

} // namespace amelia
