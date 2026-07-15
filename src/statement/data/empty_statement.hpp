#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct EmptyStatement : Expression {
  Serialize serialize() const override;
};

} // namespace amelia
