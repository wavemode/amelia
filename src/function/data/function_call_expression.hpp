#pragma once

#include "expr/data/expression.hpp"
#include "function/data/function_signature.hpp"

namespace amelia {

struct FunctionCallExpression : Expression {
  Flex<Expression> callee;
  FunctionSignature *signature;
  List<Option<Flex<Expression>>> arguments;
  Serialize serialize() const override;
};

} // namespace amelia
