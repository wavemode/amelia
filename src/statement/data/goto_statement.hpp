#pragma once

#include "expr/data/expression.hpp"
#include "util/data/text.hpp"

namespace amelia {

struct GotoStatement : Expression {
  Text label;
  Serialize serialize() const override;
};

} // namespace amelia
