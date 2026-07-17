#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct WhileStatement : Expression {
  Flex<Expression> condition;
  Flex<Expression> body;
  Serialize serialize() const override;
};

} // namespace amelia
