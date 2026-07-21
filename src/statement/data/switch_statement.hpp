#pragma once

#include "expr/data/expression.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct SwitchStatement : Expression {
  List<Flex<Expression>> case_clauses;
  Option<Flex<Expression>> default_body;
  Serialize serialize() const override;
};

} // namespace amelia
