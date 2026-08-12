#pragma once

#include "expr/data/expression.hpp"
#include "sema/data/value_binding.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct ImplicitValueBindingStatement : Expression {
  List<Flex<ValueBinding>> bindings;
  Flex<Expression> body;
  Serialize serialize() const override;
};

} // namespace amelia
