#pragma once

#include "prelude.hpp"

#include "testing/data/serialize.hpp"

#include "sema/data/binding.hpp"
#include "util/data/flex.hpp"
#include "util/data/map.hpp"

namespace amelia {

using BindingId = int32_t;

struct Scope {
  Map<Text, BindingId> active_binding_ids;
  List<Flex<Binding>> active_bindings;

  Serialize serialize() const;
};

} // namespace amelia
