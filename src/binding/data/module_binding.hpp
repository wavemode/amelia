#pragma once

#include "binding/data/binding.hpp"
#include "sema/data/scope.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct ModuleBinding : Binding {
  Option<Flex<Scope>> scope;
};

} // namespace amelia
