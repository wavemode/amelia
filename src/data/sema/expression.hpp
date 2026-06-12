#pragma once

#include "prelude.hpp"

#include "data/testing/pretty_print.hpp"
#include "data/util/flex_shared.hpp"

#include "data/sema/type.hpp"
#include "data/source/number_literal.hpp"

namespace amelia {

struct Expression {
  FlexShared<Type> type;
  NodeId node_id;

  virtual PrettyPrint pretty_print() const = 0;
  virtual ~Expression() = default;
};

struct NumberLiteralExpression : Expression {
  NumberLiteral value;

  PrettyPrint pretty_print() const override;
};

} // namespace amelia
