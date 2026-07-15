#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct ReferenceExpression : Expression {
  Flex<Expression> referent;
  bool is_const;
  bool is_move;

  Serialize serialize() const override;
};

} // namespace amelia
