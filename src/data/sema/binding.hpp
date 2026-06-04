#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"
#include "data/source/declaration_visibility.hpp"
#include "data/sema/type.hpp"

namespace amelia {

struct Binding {
  String name;
  DeclVisibility visibility;
  NodeId decl;
  FlexShared<Type> type;
};

} // namespace amelia
