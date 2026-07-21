#pragma once

#include <cstdint>

#include "binding/data/binding_kind.hpp"
#include "source/data/declaration_visibility.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

class Serialize;

using BindingId = int32_t;
using NodeId = int32_t;

struct Binding {
  NodeId decl;
  String name;
  BindingKind kind;
  DeclarationVisibility visibility = DeclarationVisibility::Default;
  Option<BindingId> id;
  Option<BindingId> shadowed_binding_id;
  List<Flex<Binding>> child_bindings;

  Serialize serialize() const;
  virtual ~Binding() = default;
};

} // namespace amelia
