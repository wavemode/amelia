#pragma once

#include "expr/data/expression.hpp"
#include "sema/data/value_binding.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct ValueBindingStatement : Expression {
  Serialize serialize() const override;
  Flex<ValueBinding> binding;
  Flex<Expression> body;
};

} // namespace amelia
