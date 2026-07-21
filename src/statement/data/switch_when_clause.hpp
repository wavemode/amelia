#pragma once

#include "expr/data/expression.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct SwitchWhenClause : Expression {
  Flex<Expression> condition;
  Flex<Expression> body;
  Serialize serialize() const override;
};

} // namespace amelia
