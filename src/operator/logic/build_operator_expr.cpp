#include "build_operator_expr.hpp"

#include "builtin/data/builtin_type.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "literal/data/identifier_expression.hpp"
#include "operator/data/binary_operator_kind.hpp"
#include "operator/data/native_binary_operation.hpp"
#include "operator/data/native_unary_operation.hpp"
#include "operator/data/unary_operator_kind.hpp"
#include "parser/data/node.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "util/data/flex.hpp"

namespace amelia {

namespace {

#define FOR_EACH_BINARY_OP                                                                         \
  X(AddExprNode)                                                                                   \
  X(SubtractExprNode)                                                                              \
  X(MultiplyExprNode)                                                                              \
  X(DivideExprNode)                                                                                \
  X(AndExprNode)                                                                                   \
  X(BitwiseAndExprNode)                                                                            \
  X(BitwiseOrExprNode)                                                                             \
  X(BitwiseXorExprNode)                                                                            \
  X(EqualsExprNode)                                                                                \
  X(GreaterExprNode)                                                                               \
  X(GreaterEqualsExprNode)                                                                         \
  X(LessExprNode)                                                                                  \
  X(LessEqualsExprNode)                                                                            \
  X(LeftShiftExprNode)                                                                             \
  X(ModuloExprNode)                                                                                \
  X(NotEqualsExprNode)                                                                             \
  X(OrExprNode)                                                                                    \
  X(RightShiftExprNode)

#define FOR_EACH_ASSIGNMENT_OP                                                                     \
  X(AssignmentStmtNode)                                                                            \
  X(AddAssignStmtNode)                                                                             \
  X(BitwiseAndAssignStmtNode)                                                                      \
  X(BitwiseOrAssignStmtNode)                                                                       \
  X(BitwiseXorAssignStmtNode)                                                                      \
  X(DivAssignStmtNode)                                                                             \
  X(LeftShiftAssignStmtNode)                                                                       \
  X(ModAssignStmtNode)                                                                             \
  X(MulAssignStmtNode)                                                                             \
  X(RightShiftAssignStmtNode)                                                                      \
  X(SubAssignStmtNode)

#define FOR_EACH_UNARY_OP                                                                          \
  X(NegateExprNode)                                                                                \
  X(PositiveExprNode)                                                                              \
  X(NotExprNode)                                                                                   \
  X(BitwiseNotExprNode)

#define FOR_EACH_INCREMENT_DECREMENT_OP                                                            \
  X(PreDecrementStmtNode)                                                                          \
  X(PostDecrementStmtNode)                                                                         \
  X(PreIncrementStmtNode)                                                                          \
  X(PostIncrementStmtNode)

void assert_mutable_operand(IModuleAnalysisState &module_state, Flex<Expression> operand) {
  if (operand->is<IdentifierExpression>()) {
    auto &identifier_expr = operand->as<IdentifierExpression>();
    if (identifier_expr.binding->kind == BindingKind::Constant) {
      String error_message = "Cannot modify constant '";
      error_message.append(identifier_expr.binding->name);
      error_message.append('\'');
      module_state.raise_type_error_at_node(operand->node_id, move(error_message));
    }
    if (identifier_expr.binding->kind != BindingKind::Variable) {
      String error_message = "Cannot assign to '";
      error_message.append(identifier_expr.binding->name);
      error_message.append("' because it is not a variable");
      module_state.raise_type_error_at_node(operand->node_id, move(error_message));
    }
  } else { // TODO: field access, deref
    module_state.raise_type_error_at_node(
        operand->node_id, "Operand of mutation must be an assignable place"
    );
  }
}

bool is_mutating_binary_op(BinaryOperatorKind op_kind) {
  switch (op_kind) {
  case BinaryOperatorKind::Assignment:
  case BinaryOperatorKind::AddAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
    return true;
  default:
    return false;
  }
}

bool is_mutating_unary_op(UnaryOperatorKind op_kind) {
  switch (op_kind) {
  case UnaryOperatorKind::Increment:
  case UnaryOperatorKind::Decrement:
    return true;
  default:
    return false;
  }
}

bool is_non_promoting_binary_op(BinaryOperatorKind op_kind) {
  switch (op_kind) {
  case BinaryOperatorKind::LeftShift:
  case BinaryOperatorKind::RightShift:
  case BinaryOperatorKind::LShiftAssignment:
  case BinaryOperatorKind::RShiftAssignment:
    return true;
  default:
    return false;
  }
}

bool is_rhs_promoting_binary_op(BinaryOperatorKind op_kind) {
  switch (op_kind) {
  case BinaryOperatorKind::Assignment:
  case BinaryOperatorKind::AddAssignment:
  case BinaryOperatorKind::SubAssignment:
  case BinaryOperatorKind::MulAssignment:
  case BinaryOperatorKind::DivAssignment:
  case BinaryOperatorKind::ModAssignment:
  case BinaryOperatorKind::BitAndAssignment:
  case BinaryOperatorKind::BitOrAssignment:
  case BinaryOperatorKind::BitXorAssignment:
    return true;
  default:
    return false;
  }
}

BinaryOperatorKind binary_op_kind_of_node_type(NodeType node_type) {
  switch (node_type) {
  case NodeType::AddExprNode:
    return BinaryOperatorKind::Add;
  case NodeType::SubtractExprNode:
    return BinaryOperatorKind::Subtract;
  case NodeType::MultiplyExprNode:
    return BinaryOperatorKind::Multiply;
  case NodeType::DivideExprNode:
    return BinaryOperatorKind::Divide;
  case NodeType::AndExprNode:
    return BinaryOperatorKind::And;
  case NodeType::BitwiseAndExprNode:
    return BinaryOperatorKind::BitwiseAnd;
  case NodeType::BitwiseOrExprNode:
    return BinaryOperatorKind::BitwiseOr;
  case NodeType::BitwiseXorExprNode:
    return BinaryOperatorKind::BitwiseXor;
  case NodeType::EqualsExprNode:
    return BinaryOperatorKind::Equals;
  case NodeType::GreaterExprNode:
    return BinaryOperatorKind::Greater;
  case NodeType::GreaterEqualsExprNode:
    return BinaryOperatorKind::GreaterEquals;
  case NodeType::LessExprNode:
    return BinaryOperatorKind::Less;
  case NodeType::LessEqualsExprNode:
    return BinaryOperatorKind::LessEquals;
  case NodeType::LeftShiftExprNode:
    return BinaryOperatorKind::LeftShift;
  case NodeType::ModuloExprNode:
    return BinaryOperatorKind::Modulo;
  case NodeType::NotEqualsExprNode:
    return BinaryOperatorKind::NotEquals;
  case NodeType::OrExprNode:
    return BinaryOperatorKind::Or;
  case NodeType::RightShiftExprNode:
    return BinaryOperatorKind::RightShift;
  case NodeType::AddAssignStmtNode:
    return BinaryOperatorKind::AddAssignment;
  case NodeType::AssignmentStmtNode:
    return BinaryOperatorKind::Assignment;
  case NodeType::BitwiseAndAssignStmtNode:
    return BinaryOperatorKind::BitAndAssignment;
  case NodeType::BitwiseOrAssignStmtNode:
    return BinaryOperatorKind::BitOrAssignment;
  case NodeType::BitwiseXorAssignStmtNode:
    return BinaryOperatorKind::BitXorAssignment;
  case NodeType::DivAssignStmtNode:
    return BinaryOperatorKind::DivAssignment;
  case NodeType::LeftShiftAssignStmtNode:
    return BinaryOperatorKind::LShiftAssignment;
  case NodeType::ModAssignStmtNode:
    return BinaryOperatorKind::ModAssignment;
  case NodeType::MulAssignStmtNode:
    return BinaryOperatorKind::MulAssignment;
  case NodeType::RightShiftAssignStmtNode:
    return BinaryOperatorKind::RShiftAssignment;
  case NodeType::SubAssignStmtNode:
    return BinaryOperatorKind::SubAssignment;
  default:
    throw RuntimeError(
        "binary_op_kind_of_node_type() called with a node type that is not a binary operator"
    );
  }
}

UnaryOperatorKind unary_op_kind_of_node_type(NodeType node_type) {
  switch (node_type) {
  case NodeType::NegateExprNode:
    return UnaryOperatorKind::Negate;
  case NodeType::PositiveExprNode:
    return UnaryOperatorKind::Positive;
  case NodeType::NotExprNode:
    return UnaryOperatorKind::Not;
  case NodeType::BitwiseNotExprNode:
    return UnaryOperatorKind::BitwiseNot;
  case NodeType::PreDecrementStmtNode:
  case NodeType::PostDecrementStmtNode:
    return UnaryOperatorKind::Decrement;
  case NodeType::PreIncrementStmtNode:
  case NodeType::PostIncrementStmtNode:
    return UnaryOperatorKind::Increment;
  default:
    throw RuntimeError(
        "unary_op_kind_of_node_type() called with a node type that is not a unary operator"
    );
  }
}

} // namespace

Flex<Expression> build_binary_operator_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const Node &node = module_state.get_node(expr_node_id);
  NodeId left_expr_node_id;
  NodeId right_expr_node_id;
  BinaryOperatorKind op_kind;
  switch (node.type()) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE: {                                                                      \
    const NODE_TYPE &expr_node = node.as_##NODE_TYPE();                                            \
    left_expr_node_id = expr_node.left;                                                            \
    right_expr_node_id = expr_node.right;                                                          \
    op_kind = binary_op_kind_of_node_type(node.type());                                            \
  } break;
    FOR_EACH_BINARY_OP
#undef X

#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE: {                                                                      \
    const NODE_TYPE &expr_node = node.as_##NODE_TYPE();                                            \
    left_expr_node_id = expr_node.target;                                                          \
    right_expr_node_id = expr_node.expr;                                                           \
    op_kind = binary_op_kind_of_node_type(node.type());                                            \
  } break;
    FOR_EACH_ASSIGNMENT_OP
#undef X
  default:
    throw RuntimeError(
        "build_binary_operator_expression() called with a node type that is not a binary operator"
    );
  }

  auto left_expr = build_expression(module_state, left_expr_node_id);
  auto original_left_type = left_expr->type;
  auto right_expr = build_expression(module_state, right_expr_node_id);
  auto original_right_type = right_expr->type;

  if (is_mutating_binary_op(op_kind)) {
    assert_mutable_operand(module_state, left_expr);
  }

  auto left_type = left_expr->type;
  auto right_type = right_expr->type;

  auto result = left_type->perform_binary_op(
      expr_node_id, op_kind, left_expr, right_type, right_expr
  );
  if (!result.has_value()) {
    left_expr->type = left_expr->type->remove_comptime_const_from_type();
    right_expr->type = right_expr->type->remove_comptime_const_from_type();
    left_type = left_expr->type;
    right_type = right_expr->type;

    result = left_type->perform_binary_op(expr_node_id, op_kind, left_expr, right_type, right_expr);
  }
  if (!is_non_promoting_binary_op(op_kind)) {
    if (!result.has_value()) {
      auto try_convert_right = left_type->coerce_expr(right_type, right_expr);
      if (try_convert_right.has_value()) {
        result = left_type->perform_binary_op(
            expr_node_id, op_kind, left_expr, left_type, move(try_convert_right.value())
        );
      }
    }
    if (!result.has_value() && !is_rhs_promoting_binary_op(op_kind)) {
      auto try_convert_left = right_type->coerce_expr(left_type, left_expr);
      if (try_convert_left.has_value()) {
        result = right_type->perform_binary_op(
            expr_node_id, op_kind, move(try_convert_left.value()), right_type, right_expr
        );
      }
    }
  }
  if (!result.has_value()) {
    String error_message = "Cannot perform binary operation '";
    serialize_binary_operator_kind(op_kind).to_string(error_message);
    error_message.append("' on types '");
    original_left_type->serialize().to_string(error_message);
    error_message.append("' and '");
    original_right_type->serialize().to_string(error_message);
    error_message.append("'");
    module_state.raise_type_error_at_node(expr_node_id, move(error_message));
  }
  return result.value();
}

Option<Flex<Expression>> perform_native_shift(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &result_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr
) {
  if (!right_type.is_integral() || !right_type.is_primitive()) {
    return None();
  }
  auto result = emplace_flex<NativeBinaryOperationExpression>();
  result->op_kind = op_kind;
  result->left = left_expr.flex();
  result->right = right_expr.flex();
  result->type = result_type.flex();
  result->node_id = expr_node_id;
  return result;
}

Option<Flex<Expression>> perform_native_binary_op(
    NodeId expr_node_id,
    BinaryOperatorKind op_kind,
    const Type &left_type,
    const Expression &left_expr,
    const Type &right_type,
    const Expression &right_expr,
    const Type &result_type
) {
  if (!left_type.unify_type(right_type)) {
    return None();
  }
  auto result = emplace_flex<NativeBinaryOperationExpression>();
  result->op_kind = op_kind;
  result->left = left_expr.flex();
  result->right = right_expr.flex();
  result->type = result_type.flex();
  result->node_id = expr_node_id;
  return result;
}

Option<Flex<Expression>> perform_native_unary_op(
    NodeId expr_node_id,
    UnaryOperatorKind op_kind,
    const Type &result_type,
    const Expression &operand_expr
) {
  auto result = emplace_flex<NativeUnaryOperationExpression>();
  result->op_kind = op_kind;
  result->operand = operand_expr.flex();
  result->type = result_type.flex();
  result->node_id = expr_node_id;
  return result;
}

Flex<Expression> build_unary_operator_expression(
    IModuleAnalysisState &module_state, NodeId expr_node_id
) {
  const Node &node = module_state.get_node(expr_node_id);
  NodeId operand_node_id;
  UnaryOperatorKind op_kind;
  switch (node.type()) {
#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE: {                                                                      \
    const NODE_TYPE &n = node.as_##NODE_TYPE();                                                    \
    operand_node_id = n.expr;                                                                      \
    op_kind = unary_op_kind_of_node_type(node.type());                                             \
    break;                                                                                         \
  }
    FOR_EACH_UNARY_OP
#undef X

#define X(NODE_TYPE)                                                                               \
  case NodeType::NODE_TYPE: {                                                                      \
    const NODE_TYPE &n = node.as_##NODE_TYPE();                                                    \
    operand_node_id = n.target;                                                                    \
    op_kind = unary_op_kind_of_node_type(node.type());                                             \
    break;                                                                                         \
  }
    FOR_EACH_INCREMENT_DECREMENT_OP
#undef X

  default:
    throw RuntimeError(
        "build_unary_operator_expression() called with a node type that is not a unary operator"
    );
  }

  auto operand_expr = build_expression(module_state, operand_node_id);

  if (is_mutating_unary_op(op_kind)) {
    assert_mutable_operand(module_state, operand_expr);
  }

  auto result = operand_expr->type->perform_unary_op(expr_node_id, op_kind, operand_expr);
  if (!result.has_value()) {
    switch (op_kind) {
    case UnaryOperatorKind::Negate:
    case UnaryOperatorKind::Positive:
    case UnaryOperatorKind::BitwiseNot:
    case UnaryOperatorKind::Decrement:
    case UnaryOperatorKind::Increment:
      // TODO: attempt numeric coercion
      break;
    case UnaryOperatorKind::Not: {
      auto coerced_operand = BOOL_TYPE->coerce_expr(operand_expr);
      if (coerced_operand.has_value()) {
        result = BOOL_TYPE->perform_unary_op(expr_node_id, op_kind, coerced_operand.value());
      }
    } break;
    }
  }

  if (!result.has_value()) {
    String error_message = "Cannot apply unary operator '";
    serialize_unary_operator_kind(op_kind).to_string(error_message);
    error_message.append("' to expression of type '");
    operand_expr->type->serialize().to_string(error_message);
    error_message.append("'");
    module_state.raise_type_error_at_node(expr_node_id, move(error_message));
  }

  return result.value();
}

} // namespace amelia
