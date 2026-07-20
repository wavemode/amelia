#pragma once

#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct IfStatement : Expression {
  Flex<Expression> condition;
  Flex<Expression> then_branch;
  Option<Flex<Expression>> else_branch;
  Serialize serialize() const override;
};

} // namespace amelia
