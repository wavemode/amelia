#pragma once

#include "expr/data/expression.hpp"
#include "util/data/text.hpp"

namespace amelia {

struct LabelStatement : Expression {
  Text name;
  Serialize serialize() const override;
};

} // namespace amelia
