#pragma once

#include "expr/data/expression.hpp"
#include "sema/data/binding.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct ValueBindingExpression : ExpressionWithDynamicId<ValueBindingExpression> {
  Serialize serialize() const override;
  Flex<ValueBinding> binding;
  Flex<Expression> body;
};

} // namespace amelia
