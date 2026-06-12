#pragma once

#include "prelude.hpp"

#include "data/testing/pretty_print.hpp"

#include "data/sema/binding.hpp"
#include "data/util/flex_shared.hpp"
#include "data/util/map.hpp"

namespace amelia {

using BindingId = int32_t;

struct Scope {
  Map<Text, BindingId> binding_ids;
  List<FlexShared<Binding>> bindings;
  Option<Ref<Scope>> parent;

  PrettyPrint pretty_print() const;
};

} // namespace amelia
