#pragma once

#include "expr/data/expression.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct StringLiteralExpression : ExpressionWithDynamicId<StringLiteralExpression> {
  String value;
  Serialize serialize() const override;
};

} // namespace amelia
