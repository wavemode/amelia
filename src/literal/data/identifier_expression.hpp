#pragma once

#include "binding/data/binding.hpp"
#include "expr/data/expression.hpp"

namespace amelia {

struct IdentifierExpression : Expression {
  Flex<Binding> binding;
  Serialize serialize() const override;
};

} // namespace amelia
