#include "build_ref_expr.hpp"

#include "expr/logic/build.hpp"
#include "literal/data/identifier_expression.hpp"
#include "parser/data/node.hpp"
#include "reference/data/reference_expression.hpp"
#include "reference/data/reference_type.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "util/data/string.hpp"

namespace amelia {

Flex<Expression> build_reference_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const auto &ref_node = module_state.get_node(expr_node_id).as_RefExprNode();
  auto referent_expr = build_expression(module_state, ref_node.expr);
  if (referent_expr->is<IdentifierExpression>()) {
    auto &binding = static_cast<IdentifierExpression &>(*referent_expr).binding;
    if (binding->kind == BindingKind::Variable || binding->kind == BindingKind::Constant) {
      auto reference_type = emplace_flex<ReferenceType>();
      reference_type->referent = referent_expr->type;
      reference_type->is_const = ref_node.is_const;
      if (!reference_type->is_const && binding->kind == BindingKind::Constant) {
        String error_message = "Cannot take mutable reference to constant '";
        error_message.append(binding->name);
        error_message.append("'");
        module_state.raise_error_at_node(expr_node_id, move(error_message));
      }
      reference_type->is_move = ref_node.is_move;
      auto result = emplace_flex<ReferenceExpression>();
      result->node_id = expr_node_id;
      result->type = reference_type;
      result->referent = referent_expr;
      result->is_const = ref_node.is_const;
      result->is_move = ref_node.is_move;
      return result;
    }
    module_state.raise_error_at_node(
        expr_node_id, "not implemented (ref of non-variable/constant identifier)"
    );
  }
  module_state.raise_error_at_node(
      expr_node_id, "not implemented (ref of non-identifier expression)"
  );
}

} // namespace amelia
