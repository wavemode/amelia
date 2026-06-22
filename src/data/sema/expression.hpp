#pragma once

#include "prelude.hpp"

#include "data/sema/builtin_kind.hpp"
#include "data/source/number_literal.hpp"
#include "data/testing/serialize.hpp"
#include "data/util/flex.hpp"
#include "data/util/integer.hpp"
#include "data/util/rational.hpp"

namespace amelia {

Serialize serialize_builtin_kind(BuiltinKind kind);

enum class TypeKind : uint8_t {
  Inferred,
  Alias,
  TypeFn,
  Apply,
  Builtin,
  BitInt,
  Tuple,
  Struct,
  Reference,
  Pointer,
  Array,
  Slice,
  Impl,
  ConstInteger,
  ConstRational,
  ConstBoolean,
  Class,
  Union,
  Concept,
  Function,
  FunctionPointer,
  Closure,
  Variable,
};

Serialize serialize_type_kind(TypeKind kind);

struct Type {
  TypeKind kind;
  Serialize serialize() const;
  virtual ~Type() = default;

protected:
  Type(TypeKind kind) : kind(kind) {}
};

enum class ExpressionKind : uint8_t {
  NumberLiteral,
  BooleanLiteral,
  NullLiteral,
  Identifier,
  UnaryOperation,
  BuiltinTypeCast,
  Sequence,
  ValueBinding,
  Empty,
  FunctionCall,
  ConstInteger,
  ConstRational,
  ConstBoolean,
};

struct Expression {
  ExpressionKind kind;
  Flex<Type> type;
  NodeId node_id;

  virtual Serialize serialize() const = 0;
  virtual ~Expression();

protected:
  Expression(ExpressionKind kind) : kind(kind) {}
};

struct InferredType : Type {
  InferredType() : Type(TypeKind::Inferred) {}
  Flex<Type> target;
};

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

struct ConstIntegerExpression : Expression {
  ConstIntegerExpression() : Expression(ExpressionKind::ConstInteger) {}
  Integer value;
  Serialize serialize() const override;
};

struct ConstRationalExpression : Expression {
  ConstRationalExpression() : Expression(ExpressionKind::ConstRational) {}
  Rational value;
  Serialize serialize() const override;
};

struct ConstBooleanExpression : Expression {
  ConstBooleanExpression() : Expression(ExpressionKind::ConstBoolean) {}
  bool value;
  Serialize serialize() const override;
};

struct NumberLiteralExpression : Expression {
  NumberLiteralExpression() : Expression(ExpressionKind::NumberLiteral) {}
  NumberLiteral value;
  Serialize serialize() const override;
};

struct BooleanLiteralExpression : Expression {
  BooleanLiteralExpression() : Expression(ExpressionKind::BooleanLiteral) {}
  bool value;
  Serialize serialize() const override;
};

struct NullLiteralExpression : Expression {
  NullLiteralExpression() : Expression(ExpressionKind::NullLiteral) {}
  Serialize serialize() const override;
};

struct IdentifierExpression : Expression {
  IdentifierExpression() : Expression(ExpressionKind::Identifier) {}
  String name;
  Serialize serialize() const override;
};

enum class UnaryOperatorKind : uint8_t { Negate };

Serialize serialize_unary_operator_kind(UnaryOperatorKind kind);

struct UnaryOperationExpression : Expression {
  UnaryOperationExpression() : Expression(ExpressionKind::UnaryOperation) {}
  UnaryOperatorKind op_kind;
  Flex<Expression> operand;
  Serialize serialize() const override;
};

struct BuiltinTypeCastExpression : Expression {
  BuiltinTypeCastExpression() : Expression(ExpressionKind::BuiltinTypeCast) {}
  Flex<Expression> expr;
  Serialize serialize() const override;
};

struct SequenceExpression : Expression {
  SequenceExpression() : Expression(ExpressionKind::Sequence) {}
  Serialize serialize() const override;
  List<Flex<Expression>> exprs;
};

struct ValueBindingExpression : Expression {
  ValueBindingExpression() : Expression(ExpressionKind::ValueBinding) {}
  Serialize serialize() const override;
  String name;
  Option<Flex<Type>> binding_type;
  Option<Flex<Expression>> binding_value;
  Option<Flex<Expression>> body;
};

struct EmptyExpression : Expression {
  EmptyExpression() : Expression(ExpressionKind::Empty) {}
  Serialize serialize() const override;
};

struct ReturnExpression : Expression {
  ReturnExpression() : Expression(ExpressionKind::Empty) {}
  Option<Flex<Expression>> value;
  Serialize serialize() const override;
};

struct FunctionCallExpression : Expression {
  FunctionCallExpression() : Expression(ExpressionKind::FunctionCall) {}
  Flex<Expression> callee;
  FunctionType::Signature *signature;
  List<Option<Flex<Expression>>> arguments;
  Serialize serialize() const override;
};

} // namespace amelia
