#pragma once

#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"

namespace amelia {

class Serialize;

struct StatementSequence : ExpressionWithDynamicId<StatementSequence> {
  Serialize serialize() const override;
  List<Flex<Expression>> stmts;
};

} // namespace amelia
