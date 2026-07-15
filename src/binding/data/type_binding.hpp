#pragma once

#include "binding/data/binding.hpp"
#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct TypeBinding : Binding {
  Option<Flex<Type>> type;
};

} // namespace amelia
