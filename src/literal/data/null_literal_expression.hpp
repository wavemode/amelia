#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct NullLiteralExpression : ExpressionWithDynamicId<NullLiteralExpression> {
  Serialize serialize() const override;
};

} // namespace amelia
