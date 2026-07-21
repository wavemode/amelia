#pragma once

#include "binding/data/value_binding.hpp"
#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct ImplicitValueBindingStatement : Expression {
  List<Flex<ValueBinding>> bindings;
  Flex<Expression> body;
  Serialize serialize() const override;
};

} // namespace amelia
