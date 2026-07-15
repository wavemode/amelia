#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct CharLiteralExpression : Expression {
  uint32_t value;
  Serialize serialize() const override;
};

} // namespace amelia
