#pragma once

#include "binding/data/value_binding.hpp"
#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct ValueBindingStatement : ExpressionWithDynamicId<ValueBindingStatement> {
  Serialize serialize() const override;
  Flex<ValueBinding> binding;
  Flex<Expression> body;
};

} // namespace amelia
