#pragma once

#include "util/data/flex.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

#include "expr/data/expression.hpp"

namespace amelia {

class Serialize;

struct FunctionParameter {
  String name;
  Flex<Type> type;
  Option<Flex<Expression>> default_value;
};

Serialize serialize_function_parameter(const FunctionParameter &parameter);

} // namespace amelia
