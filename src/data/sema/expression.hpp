#pragma once

#include "prelude.hpp"

#include "data/testing/serialize.hpp"
#include "data/util/flex.hpp"

namespace amelia {

struct Type;

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

} // namespace amelia
