#pragma once

#include "expr/data/expression.hpp"
#include "sema/data/binding.hpp"
#include "util/data/flex.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct TypeBindingExpression : ExpressionWithDynamicId<TypeBindingExpression> {
  Serialize serialize() const override;
  String name;
  Flex<TypeBinding> binding;
  Flex<Expression> body;
};

} // namespace amelia
