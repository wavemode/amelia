#include "type_conversion.hpp"

#include "alias/data/alias_type.hpp"
#include "expr/data/expression.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "tuple/data/tuple_type.hpp"
#include "type/data/native_type_cast_expression.hpp"
#include "util/data/string.hpp"

namespace amelia {

Flex<Expression> native_type_cast(const Type &target_type, const Expression &expr) {
  auto coerce_expr = emplace_flex<NativeTypeCastExpression>();
  coerce_expr->type = target_type.flex();
  coerce_expr->expr = expr.flex();
  return coerce_expr;
}

} // namespace amelia
