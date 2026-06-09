#pragma once

#include "prelude.hpp"

#include "data/sema/binding.hpp"
#include "data/util/flex_shared.hpp"
#include "data/util/map.hpp"

namespace amelia {

using BindingId = int32_t;

struct Scope {
  Map<Text, BindingId> binding_ids;
  Deque<Binding> bindings; // this is a Deque so that the contents never move
  Option<Ref<Scope>> parent;
};

} // namespace amelia
