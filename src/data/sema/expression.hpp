#pragma once

#include "prelude.hpp"

#include "data/sema/builtin_kind.hpp"
#include "data/source/number_literal.hpp"
#include "data/testing/serialize.hpp"
#include "data/util/flex.hpp"
#include "data/util/integer.hpp"
#include "data/util/rational.hpp"

namespace amelia {

enum class TypeKind : uint8_t {
  Alias,
  Reference,
  Struct,
  Tuple,
  Array,
  TypeFn,
  Apply,
  Builtin,
  BitInt,
  Pointer,
  Slice,
  Impl,
  ConstInteger,
  ConstRational,
  ConstBoolean,
  ConstCharacter,
  ConstString,
  Class,
  Union,
  Concept,
  Function,
  FunctionPointer,
  Closure,
  Variable,
};

Serialize serialize_type_kind(TypeKind kind);

struct Expression;
struct ValueBinding;
struct TypeBinding;

struct Type {
  TypeKind kind;

  virtual Serialize serialize() const = 0;
  virtual ~Type() = default;

protected:
  Type(TypeKind kind) : kind(kind) {}
};

enum class ExpressionKind : uint8_t {
  NumberLiteral,
  BooleanLiteral,
  NullLiteral,
  CharLiteral,
  StringLiteral,
  Identifier,
  UnaryOperation,
  BinaryOperation,
  BuiltinTypeCast,
  Sequence,
  TypeBinding,
  ValueBinding,
  Empty,
  Return,
  FunctionCall,
  ConstInteger,
  ConstRational,
  ConstBoolean,
  AddressOf,
  Tuple,
  ArrayLiteral,
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

////////// Type variants //////////

struct BuiltinType : Type {
  BuiltinType() : Type(TypeKind::Builtin) {}
  BuiltinKind builtin_kind;

  Serialize serialize() const override;
};

struct AliasType : Type {
  AliasType() : Type(TypeKind::Alias) {}
  String name;
  String module_name;
  Flex<Type> target;

  Serialize serialize() const override;
};

struct ReferenceType : Type {
  ReferenceType() : Type(TypeKind::Reference) {}
  Flex<Type> referent;
  bool is_const;
  bool is_move;

  Serialize serialize() const override;
};

struct PointerType : Type {
  PointerType() : Type(TypeKind::Pointer) {}
  Flex<Type> pointee;
  bool is_const;

  Serialize serialize() const override;
};

struct SliceType : Type {
  SliceType() : Type(TypeKind::Slice) {}
  Flex<Type> element_type;

  Serialize serialize() const override;
};

struct ArrayType : Type {
  ArrayType() : Type(TypeKind::Array) {}
  Flex<Type> element_type;
  uint64_t size;

  Serialize serialize() const override;
};

struct ConstIntegerType : Type {
  ConstIntegerType() : Type(TypeKind::ConstInteger) {}
  ConstIntegerType(Integer value) : Type(TypeKind::ConstInteger), value(move(value)) {}
  Integer value;

  Serialize serialize() const override;
};

struct ConstRationalType : Type {
  ConstRationalType() : Type(TypeKind::ConstRational) {}
  ConstRationalType(Rational value) : Type(TypeKind::ConstRational), value(move(value)) {}
  Rational value;

  Serialize serialize() const override;
};

struct ConstBooleanType : Type {
  ConstBooleanType() : Type(TypeKind::ConstBoolean) {}
  ConstBooleanType(bool value) : Type(TypeKind::ConstBoolean), value(value) {}
  bool value;

  Serialize serialize() const override;
};

struct ConstCharacterType : Type {
  ConstCharacterType() : Type(TypeKind::ConstCharacter) {}
  ConstCharacterType(uint32_t value) : Type(TypeKind::ConstCharacter), value(value) {}
  uint32_t value;

  Serialize serialize() const override;
};

struct ConstStringType : Type {
  ConstStringType() : Type(TypeKind::ConstString) {}
  ConstStringType(String value) : Type(TypeKind::ConstString), value(move(value)) {}
  String value;

  Serialize serialize() const override;
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

  Serialize serialize() const override;
};

struct TupleType : Type {
  TupleType() : Type(TypeKind::Tuple) {}
  List<Flex<Type>> element_types;

  Serialize serialize() const override;
};

struct BitIntType : Type {
  BitIntType() : Type(TypeKind::BitInt) {}
  uint32_t bit_width;
  bool is_signed;

  Serialize serialize() const override;
};

////////// Expression variants //////////

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

struct CharLiteralExpression : Expression {
  CharLiteralExpression() : Expression(ExpressionKind::CharLiteral) {}
  uint32_t value;
  Serialize serialize() const override;
};

struct StringLiteralExpression : Expression {
  StringLiteralExpression() : Expression(ExpressionKind::StringLiteral) {}
  String value;
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

enum class BinaryOperatorKind : uint8_t { Add, Subtract, Multiply, Divide };

Serialize serialize_binary_operator_kind(BinaryOperatorKind kind);

struct BuiltinBinaryOperationExpression : Expression {
  BuiltinBinaryOperationExpression() : Expression(ExpressionKind::BinaryOperation) {}
  BinaryOperatorKind op_kind;
  Flex<Expression> left;
  Flex<Expression> right;
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
  Flex<ValueBinding> binding;
  Flex<Expression> body;
};

struct TypeBindingExpression : Expression {
  TypeBindingExpression() : Expression(ExpressionKind::TypeBinding) {}
  Serialize serialize() const override;
  String name;
  Flex<TypeBinding> binding;
  Flex<Expression> body;
};

struct EmptyExpression : Expression {
  EmptyExpression() : Expression(ExpressionKind::Empty) {}
  Serialize serialize() const override;
};

struct ReturnExpression : Expression {
  ReturnExpression() : Expression(ExpressionKind::Return) {}
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

struct AddressOfExpression : Expression {
  AddressOfExpression() : Expression(ExpressionKind::AddressOf) {}
  Flex<Expression> operand;
  Serialize serialize() const override;
};

struct TupleExpression : Expression {
  TupleExpression() : Expression(ExpressionKind::Tuple) {}
  List<Flex<Expression>> elements;
  Serialize serialize() const override;
};

struct ArrayLiteralExpression : Expression {
  ArrayLiteralExpression() : Expression(ExpressionKind::ArrayLiteral) {}
  List<Flex<Expression>> elements;
  Serialize serialize() const override;
};

} // namespace amelia
