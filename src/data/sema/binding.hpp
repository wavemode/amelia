#pragma once

#include "prelude.hpp"

#include "data/sema/type.hpp"
#include "data/source/declaration_visibility.hpp"
#include "data/util/flex_shared.hpp"

namespace amelia {

enum class BindingKind { Variable, Constant, Function, Type, Class, Concept, Module };

struct Binding {
  String name;
  DeclarationVisibility visibility;
  NodeId decl;
  BindingKind kind;
  FlexShared<Type> type;
};

} // namespace amelia
