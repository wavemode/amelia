#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct NullLiteralExpression : Expression {
  Serialize serialize() const override;
};

} // namespace amelia
