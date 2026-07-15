#pragma once

#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct ReturnStatement : ExpressionWithDynamicId<ReturnStatement> {
  Option<Flex<Expression>> value;
  Serialize serialize() const override;
};

} // namespace amelia
