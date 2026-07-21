#pragma once

#include "expr/data/expression.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct SwitchCaseClause : Expression {
  Option<Flex<Expression>> condition;
  Option<Flex<Expression>> expr_body;
  Option<Flex<Expression>> when_body;
  Serialize serialize() const override;
};

} // namespace amelia
