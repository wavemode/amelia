#pragma once

#include "function/data/function_parameter.hpp"
#include "function/data/function_signature.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct FunctionDefinition {
  Flex<FunctionSignature> signature;
  Flex<Expression> body;
};

Serialize serialize_function_definition(const FunctionDefinition &signature);

} // namespace amelia
