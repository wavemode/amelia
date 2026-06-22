#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex.hpp"

#include "data/sema/expression.hpp"
#include "data/source/declaration_visibility.hpp"

namespace amelia {

struct Scope;

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
