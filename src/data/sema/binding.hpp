#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"

#include "data/sema/expression.hpp"
#include "data/sema/type.hpp"
#include "data/source/declaration_visibility.hpp"

namespace amelia {

struct Scope;

enum class BindingKind : unsigned char {
  Variable,
  Constant,
  Function,
  Type,
  Class,
  Concept,
  Module
};

struct Binding {
  NodeId decl;
  BindingKind kind;
  DeclarationVisibility visibility;
  Option<FlexShared<Binding>> shadowed_binding;

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

void format_binding(AbstractString &out, const Binding &binding);

} // namespace amelia
