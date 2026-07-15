#pragma once

#include "util/data/flex.hpp"

#include "function/data/function_signature.hpp"
#include "type/data/type.hpp"

namespace amelia {

struct FunctionPointerType : Type {
  FunctionPointerType();
  Flex<FunctionSignature> signature;
};

} // namespace amelia
