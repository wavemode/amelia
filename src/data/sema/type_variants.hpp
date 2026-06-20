#pragma once

#include "prelude.hpp"

#include "data/sema/expression.hpp"
#include "data/sema/type.hpp"
#include "data/testing/serialize.hpp"
#include "data/util/flex.hpp"
#include "data/util/integer.hpp"
#include "data/util/rational.hpp"

namespace amelia {

struct BuiltinType : Type {
  BuiltinType() : Type(TypeKind::Builtin) {}
  BuiltinKind builtin_kind;
};

struct AliasType : Type {
  AliasType() : Type(TypeKind::Alias) {}
  String name;
  String module_name;
  Flex<Type> target;
};

struct ConstIntegerType : Type {
  ConstIntegerType() : Type(TypeKind::ConstInteger) {}
  ConstIntegerType(Integer value) : Type(TypeKind::ConstInteger), value(move(value)) {}
  Integer value;
};

struct ConstRationalType : Type {
  ConstRationalType() : Type(TypeKind::ConstRational) {}
  ConstRationalType(Rational value) : Type(TypeKind::ConstRational), value(move(value)) {}
  Rational value;
};

struct ConstBooleanType : Type {
  ConstBooleanType() : Type(TypeKind::ConstBoolean) {}
  ConstBooleanType(bool value) : Type(TypeKind::ConstBoolean), value(value) {}
  bool value;
};

struct FunctionType : Type {
  struct Parameter {
    String name;
    Flex<Type> type;
    Option<Flex<Expression>> default_value;
  };

  struct Signature {
    List<Parameter> parameters;
    Flex<Type> return_type;

    Serialize serialize() const;
  };

  FunctionType() : Type(TypeKind::Function) {}

  String name;
  List<Signature> signatures;

  // TODO: implicit params
  // TODO: variadic
  // TODO: generic
};

} // namespace amelia
