#pragma once

#include <cstddef>

#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/option.hpp"

namespace amelia {

struct FunctionCallExpression : Expression {
  Flex<Expression> callee;
  size_t signature_id;
  List<Option<Flex<Expression>>> arguments;
  Serialize serialize() const override;
};

} // namespace amelia
