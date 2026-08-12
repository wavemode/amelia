#pragma once

#include <cstdint>

#include "sema/data/binding_kind.hpp"
#include "source/data/declaration_visibility.hpp"
#include "type/data/type.hpp"
#include "util/data/flex.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

class Serialize;

struct Binding {
  String name;
  BindingKind kind;
  Flex<Type> type;
  DeclarationVisibility visibility = DeclarationVisibility::Default;
  Option<Flex<Binding>> shadowed_binding;

  Serialize serialize() const;
  virtual ~Binding() = default;
};

} // namespace amelia
