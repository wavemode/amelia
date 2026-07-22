#pragma once

#include <cstddef>

#include "expr/data/expression.hpp"
#include "util/data/flex.hpp"
#include "util/data/list.hpp"
#include "util/data/map.hpp"
#include "util/data/option.hpp"
#include "util/data/text.hpp"

namespace amelia {

struct FunctionCallExpression : Expression {
  Flex<Expression> callee;
  size_t signature_id;
  List<Option<Flex<Expression>>> arguments;
  Map<Text, Option<Flex<Expression>>> implicit_arguments;
  Serialize serialize() const override;
};

} // namespace amelia
