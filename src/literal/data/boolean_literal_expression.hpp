#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct BooleanLiteralExpression : ExpressionWithDynamicId<BooleanLiteralExpression> {
  bool value;
  Serialize serialize() const override;
};

} // namespace amelia
