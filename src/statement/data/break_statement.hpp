#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct BreakStatement : Expression {
  Serialize serialize() const override;
};

} // namespace amelia
