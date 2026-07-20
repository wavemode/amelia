#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct ContinueStatement : Expression {
  Serialize serialize() const override;
};

} // namespace amelia
