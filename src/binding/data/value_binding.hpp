#pragma once

#include "binding/data/binding.hpp"
#include "expr/data/expression.hpp"
#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct ValueBinding : Binding {
  Option<Flex<Type>> type;
  Option<Flex<Expression>> value;
};

} // namespace amelia
