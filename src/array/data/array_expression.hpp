#pragma once

#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"

namespace amelia {

struct ArrayLiteralExpression : Expression {
  Serialize serialize() const override;
  List<Flex<Expression>> elements;
};

} // namespace amelia
