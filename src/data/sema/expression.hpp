#pragma once

#include "prelude.hpp"

#include "data/util/flex_shared.hpp"

#include "data/sema/type.hpp"
#include "data/source/number_literal.hpp"

namespace amelia {

struct Expression {
  FlexShared<Type> type;
  NodeId node_id;

  virtual ~Expression() = default;
};

struct NumberLiteralExpression : Expression {
  NumberLiteral value;
};

void format_expression(AbstractString &out, const Expression &expr);

} // namespace amelia
