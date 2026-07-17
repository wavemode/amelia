#include <climits>

#include "analysis.hpp"

#include "array/data/array_type.hpp"
#include "array/logic/build_array_expr.hpp"
#include "binding/data/type_binding.hpp"
#include "binding/logic/analysis.hpp"
#include "bitint/data/bitint_type.hpp"
#include "builtin/data/builtin_type.hpp"
#include "const/data/const_boolean_type.hpp"
#include "const/data/const_character_type.hpp"
#include "const/data/const_integer_type.hpp"
#include "const/data/const_rational_type.hpp"
#include "const/data/const_string_type.hpp"
#include "expr/data/expression.hpp"
#include "expr/logic/build.hpp"
#include "parser/data/node.hpp"
#include "pointer/data/pointer_type.hpp"
#include "reference/data/reference_type.hpp"
#include "sema/interface/module_analysis_state.hpp"
#include "slice/data/slice_type.hpp"
#include "tuple/data/tuple_type.hpp"
#include "util/data/flex.hpp"
#include "util/data/serialize.hpp"

namespace amelia {

namespace {

Flex<Type> evaluate_type_expr_array(
    IModuleAnalysisState &module_state, NodeId expr_node_id, const ArrayExprNode &array_node
) {
  if (array_node.elements.has_value()) {
    module_state.raise_type_error_at_node(
        expr_node_id, "Invalid type expression (array type with braces)"
    );
  }

  if (array_node.infer_size) {
    module_state.raise_type_error_at_node(
        expr_node_id, "not implemented (array type expression with inferred size)"
    );
  }

  if (array_node.size.has_value()) {
    if (array_node.is_const) {
      module_state.raise_type_error_at_node(
          expr_node_id, "Invalid type expression (array type w/ const designator)"
      );
    }

    uint64_t size_value = evaluate_array_size_expr(module_state, array_node.size.value());

    auto result = emplace_flex<ArrayType>();
    result->element_type = evaluate_type_expr(module_state, array_node.type_expr);
    result->size = size_value;
    return result;
  } else {
    auto result = emplace_flex<SliceType>();
    result->element_type = evaluate_type_expr(module_state, array_node.type_expr);
    result->is_const = array_node.is_const;
    return result;
  }
}

Flex<Type> evaluate_type_expr_deref(
    IModuleAnalysisState &module_state, const DerefExprNode &deref_node
) {
  auto pointed_type = evaluate_type_expr(module_state, deref_node.expr);
  auto result = emplace_flex<PointerType>();
  result->pointee = pointed_type;
  result->is_const = deref_node.is_const;
  return result;
}

Flex<Type> evaluate_type_expr_indexing(
    IModuleAnalysisState &module_state, const IndexingExprNode &indexing_node
) {
  const auto &object_node = module_state.get_node(indexing_node.object);

  if (object_node.type() != NodeType::BitIntTypeNode) {
    module_state.raise_type_error_at_node(
        indexing_node.object, "not implemented (indexing of non-bitint type)"
    );
  }

  if (indexing_node.indices.size() != 1) {
    module_state.raise_type_error_at_node(
        indexing_node.object, "not implemented (type expr indexing with multiple indices)"
    );
  }

  const auto &index_node = module_state.get_node(indexing_node.indices[0]).as_IndexNode();
  if (index_node.name.has_value()) {
    module_state.raise_type_error_at_node(
        indexing_node.indices[0], "not implemented (type expr indexing with named index)"
    );
  }

  auto index_expr = build_expression(module_state, index_node.value);

  if (!index_expr->type->is<ConstIntegerType>()) {
    String error_message = "Expected an integer constant, but got an expression of type '";
    index_expr->type->serialize().to_string(error_message);
    error_message.append('\'');
    module_state.raise_type_error_at_node(indexing_node.indices[0], move(error_message));
  }

  const Integer &index_value = static_cast<const ConstIntegerType &>(*index_expr->type).value;
  if (index_value < 1 || index_value > UINT32_MAX) {
    String error_message = "BitInt bit-count must be a positive integer less than or equal to";
    Serialize::of(int64_t(UINT32_MAX)).to_string(error_message);
    module_state.raise_type_error_at_node(indexing_node.indices[0], move(error_message));
  }
  bool is_signed = object_node.as_BitIntTypeNode().is_signed;

  auto result = emplace_flex<BitIntType>();
  result->is_signed = is_signed;
  result->bit_width = index_value.to_uint32();
  return result;
}

Flex<Type> evaluate_type_expr_paren(
    IModuleAnalysisState &module_state, const ParenthesizedExprNode &paren_node
) {
  if (paren_node.exprs.size() == 0) {
    return NULL_TYPE;
  } else if (paren_node.exprs.size() == 1) {
    return evaluate_type_expr(module_state, paren_node.exprs[0]);
  } else {
    auto tuple_type = emplace_flex<TupleType>();
    for (NodeId sub_expr_node_id : paren_node.exprs) {
      tuple_type->element_types.push_back(evaluate_type_expr(module_state, sub_expr_node_id));
    }
    return tuple_type;
  }
}

Flex<Type> evaluate_type_expr_ref(
    IModuleAnalysisState &module_state, const RefExprNode &ref_expr_node
) {
  auto referent_type = evaluate_type_expr(module_state, ref_expr_node.expr);
  auto result = emplace_flex<ReferenceType>();
  result->referent = referent_type;
  result->is_const = ref_expr_node.is_const;
  result->is_move = ref_expr_node.is_move;
  return result;
}

Flex<Type> evaluate_type_expr_const(
    IModuleAnalysisState &module_state, const ConstTypeExprNode &const_type_expr_node
) {
  auto expr = build_expression(module_state, const_type_expr_node.expr);
  if (expr->type->is<ConstIntegerType>() || expr->type->is<ConstRationalType>() ||
      expr->type->is<ConstBooleanType>() || expr->type->is<ConstStringType>() ||
      expr->type->is<ConstCharacterType>()) {
    return expr->type;
  }

  String error_message = "Expected a constant, but got an expression of type '";
  expr->type->serialize().to_string(error_message);
  error_message.append('\'');
  module_state.raise_type_error_at_node(expr->node_id, move(error_message));
}

Flex<Type> evaluate_type_expr_builtin(const BuiltinTypeNode &builtin_type_node) {
  switch (builtin_type_node.kind) {
  case BuiltinKind::Bool:
    return BOOL_TYPE;
  case BuiltinKind::Byte:
    return BYTE_TYPE;
  case BuiltinKind::Short:
    return SHORT_TYPE;
  case BuiltinKind::Int:
    return INT_TYPE;
  case BuiltinKind::Long:
    return LONG_TYPE;
  case BuiltinKind::UByte:
    return UBYTE_TYPE;
  case BuiltinKind::UShort:
    return USHORT_TYPE;
  case BuiltinKind::UInt:
    return UINT_TYPE;
  case BuiltinKind::ULong:
    return ULONG_TYPE;
  case BuiltinKind::USize:
    return USIZE_TYPE;
  case BuiltinKind::Float:
    return FLOAT_TYPE;
  case BuiltinKind::Double:
    return DOUBLE_TYPE;
  case BuiltinKind::Char:
    return CHAR_TYPE;
  case BuiltinKind::Str:
    return STR_TYPE;
  case BuiltinKind::Null:
    return NULL_TYPE;
  case BuiltinKind::Never:
    return NEVER_TYPE;
  default:
    throw RuntimeError("unreachable");
  }
}

} // namespace

Flex<Type> evaluate_type_expr(IModuleAnalysisState &module_state, NodeId type_expr_node_id) {
  const auto &type_expr_node = module_state.get_node(type_expr_node_id);
  switch (type_expr_node.type()) {
  case NodeType::IdentifierNode:
    return resolve_type_binding(
               module_state, type_expr_node_id, type_expr_node.as_IdentifierNode().name
    )
        ->type.value()
        .weak();
  case NodeType::BuiltinTypeNode:
    return evaluate_type_expr_builtin(type_expr_node.as_BuiltinTypeNode());
  case NodeType::ConstTypeExprNode:
    return evaluate_type_expr_const(module_state, type_expr_node.as_ConstTypeExprNode());
  case NodeType::RefExprNode:
    return evaluate_type_expr_ref(module_state, type_expr_node.as_RefExprNode());
  case NodeType::ParenthesizedExprNode:
    return evaluate_type_expr_paren(module_state, type_expr_node.as_ParenthesizedExprNode());
  case NodeType::IndexingExprNode:
    return evaluate_type_expr_indexing(module_state, type_expr_node.as_IndexingExprNode());
  case NodeType::DerefExprNode:
    return evaluate_type_expr_deref(module_state, type_expr_node.as_DerefExprNode());
  case NodeType::ArrayExprNode:
    return evaluate_type_expr_array(
        module_state, type_expr_node_id, type_expr_node.as_ArrayExprNode()
    );
  default:
    module_state.raise_type_error_at_node(type_expr_node_id, "not implemented (unknown type expr)");
  }
}

} // namespace amelia
