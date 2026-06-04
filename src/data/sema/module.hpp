#pragma once

#include "prelude.hpp"

#include "data/sema/scope.hpp"
#include "data/util/map.hpp"

namespace amelia {

struct Module {
  String name;
  Scope scope;
  NodeId ast;
};

} // namespace amelia
