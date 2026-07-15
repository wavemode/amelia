#pragma once

#include <cstdint>

#include "expr/data/expression.hpp"
#include "source/data/declaration_visibility.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"
#include "util/data/string.hpp"

namespace amelia {

struct Scope;
class Serialize;

using BindingId = int32_t;

enum class BindingKind : uint8_t { Variable, Constant, Function, Type, Class, Concept, Module };

Serialize serialize_binding_kind(BindingKind kind);

struct Binding {
  NodeId decl;
  String name;
  BindingKind kind;
  DeclarationVisibility visibility;
  Option<BindingId> id;
  Option<BindingId> shadowed_binding_id;
  List<Flex<Binding>> child_bindings;

  Serialize serialize() const;
  virtual ~Binding() = default;
};

struct ValueBinding : Binding {
  Option<Flex<Type>> type;
  Option<Flex<Expression>> value;
};

struct TypeBinding : Binding {
  Option<Flex<Type>> type;
};

struct ModuleBinding : Binding {
  Option<Flex<Scope>> scope;
};

} // namespace amelia
