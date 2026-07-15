#pragma once

#include "binding/data/type_binding.hpp"
#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct TypeBindingStatement : Expression {
  Serialize serialize() const override;
  String name;
  Flex<TypeBinding> binding;
  Flex<Expression> body;
};

} // namespace amelia
