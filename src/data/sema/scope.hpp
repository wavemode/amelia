#pragma once

#include "prelude.hpp"

#include "data/util/map.hpp"
#include "data/sema/binding.hpp"

namespace amelia {

using BindingId = int32_t;

struct Scope {
  Map<Text, BindingId> binding_ids;
  List<Binding> bindings;
  Option<Scope*> parent;
};

} // namespace amelia
