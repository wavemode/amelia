#pragma once

#include "binding/data/binding.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/text.hpp"

namespace amelia {

class Serialize;

using BindingId = int32_t;

struct Scope {
  Map<Text, BindingId> active_binding_ids;
  Map<Text, BindingId> implicit_binding_ids;
  List<Flex<Binding>> bindings;

  Serialize serialize() const;
};

} // namespace amelia
