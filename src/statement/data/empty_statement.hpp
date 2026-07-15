#pragma once

#include "expr/data/expression.hpp"

namespace amelia {

struct EmptyStatement : ExpressionWithDynamicId<EmptyStatement> {
  Serialize serialize() const override;
};

} // namespace amelia
