#pragma once

#include "util/data/flex.hpp"
#include "util/data/string.hpp"

#include "function/data/function_definition.hpp"
#include "type/data/type.hpp"

namespace amelia {

struct FunctionType : Type {
  String name;
  List<Flex<FunctionDefinition>> definitions;

  // TODO: implicit params
  // TODO: variadic
  // TODO: generic

  Serialize serialize() const override;
};

} // namespace amelia
