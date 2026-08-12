#pragma once

#include "sema/data/binding.hpp"
#include "util/data/flex.hpp"

namespace amelia {

struct Scope;

struct BindingRef {
  Flex<Binding> binding;
  Scope &scope;
};

} // namespace amelia
