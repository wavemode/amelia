#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"

#include "data/sema/binding.hpp"
#include "data/util/flex.hpp"
#include "data/util/map.hpp"

namespace amelia {

using BindingId = int32_t;

struct Scope {
  Map<Text, BindingId> binding_ids;
  List<Flex<Binding>> bindings;
  Option<Ref<Scope>> parent;

  Serialize serialize() const;
};

} // namespace amelia
