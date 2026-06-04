#pragma once

#include "prelude.hpp"

#include "data/util/map.hpp"
#include "data/sema/scope.hpp"

namespace amelia {

struct Module {
  String name;
  Scope scope;
  NodeId ast;
};

} // namespace amelia
