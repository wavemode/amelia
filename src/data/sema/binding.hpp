#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex_shared.hpp"

#include "data/sema/expression.hpp"
#include "data/sema/type.hpp"
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
  Option<BindingId> shadowed_binding;
  bool analyzed;
  bool top_level;

  Serialize serialize() const;
  virtual ~Binding() = default;
};

struct ValueBinding : Binding {
  Option<FlexShared<Type>> type;
  Option<FlexShared<Expression>> value;
};

struct TypeBinding : Binding {
  Option<FlexShared<Type>> type;
};

struct ModuleBinding : Binding {
  Option<FlexShared<Scope>> scope;
};

} // namespace amelia
