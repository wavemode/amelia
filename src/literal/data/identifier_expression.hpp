#pragma once

#include "expr/data/expression.hpp"
#include "sema/data/binding.hpp"

namespace amelia {

struct IdentifierExpression : ExpressionWithDynamicId<IdentifierExpression> {
  Flex<Binding> binding;
  Serialize serialize() const override;
};

} // namespace amelia
