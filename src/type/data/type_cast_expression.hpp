#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct TypeCastExpression : Expression {
  Flex<Expression> expr;
  Serialize serialize() const override;
};

} // namespace amelia
