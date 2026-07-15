#pragma once

#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"

#include "function/data/function_parameter.hpp"
#include "type/data/type.hpp"

namespace amelia {

struct FunctionSignature {
  List<FunctionParameter> parameters;
  Flex<Type> return_type;
};

Serialize serialize_function_signature(const FunctionSignature &signature);

} // namespace amelia
