#pragma once

#include "expr/data/expression.hpp"
#include "source/data/number_literal.hpp"

namespace amelia {

struct NumberLiteralExpression : ExpressionWithDynamicId<NumberLiteralExpression> {
  NumberLiteral value;
  Serialize serialize() const override;
};

} // namespace amelia
