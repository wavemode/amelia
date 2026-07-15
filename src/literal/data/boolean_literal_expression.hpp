#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct BooleanLiteralExpression : Expression {
  bool value;
  Serialize serialize() const override;
};

} // namespace amelia
